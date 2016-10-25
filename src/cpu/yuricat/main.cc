#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/base.h"
#include "core/plan/plan.h"
#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/frame_request.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/score.h"
#include "solver/endless.h"
#include "solver/puyop.h"

#include "puyo.hpp"

using namespace std;
using namespace Yuricat;

DEFINE_int32(random_seed, 1, "random seed");
DEFINE_int32(loop, 0, "the number of loops");

// グローバル変数
bool g_endlessMode = false;
bool searching = false;
std::atomic<int> g_enemySearchNum;
int64_t g_limitTime;

vector<thread> g_searchThread;
atomic<int> g_pendingScore[2]; // 溜まっている点
int g_requiredFrame[20]; // 各連鎖数に対して、必要なフレーム数

using FrameScore = tuple<int, int>;

int g_minChain[2] = {0}; // {自分, 相手} は少なくともこの連鎖はできる
int g_minScore[2] = {0}; // {自分, 相手} は少なくともこの点は取れる

tuple<int, int> g_expectedScoreDistribution[2]; // 連鎖数ごとの期待される得点

vector<tuple<int, int>> g_chainIgnitedRensaFrameScore[2][20]; // {自分, 相手} の各連鎖数に対して達成する(フレーム数, 得点)
vector<tuple<int, int>> g_ignitedRensaFrameScore[2]; // {自分, 相手} に対して達成する(フレーム数, 得点)
vector<tuple<int, int>> g_chainTotalRensaFrameScore[2][20]; // {自分, 相手} の各連鎖数に対して連鎖完了する(フレーム数, 得点)
vector<tuple<int, int>> g_totalRensaFrameScore[2]; // {自分, 相手} に対して連鎖完了する(フレーム数, 得点)

vector<int> g_requestFrame[2]; // 行動リクエストが来たフレーム数

void setEndlessMode(){ g_endlessMode = true; }
bool isEndlessMode(){ return g_endlessMode; }

#include "mate.hpp"
#include "evaluation.hpp"
#include "solo_search.hpp"

int64_t timeManage(const PlayerState& me, const PlayerState& enemy, bool fast){
    // 行動決定にかけていい時間を決める(ms単位)
    UNUSED_VARIABLE(enemy);
    
    int highest_y = -1;
    for(int x = 1; x <= FieldConstant::WIDTH; ++x){
        highest_y = max(highest_y, me.field.height(x) - abs(x - 3));
    }
    
    return fast ? 30 : min(30 + 560 * sqrt(1 - me.field.countPuyos() / 78.0), 30 + sqrt(max(0.0001, (13.0 - highest_y) / 13)) * 560);
}

Decision searchEscapeMove(const CoreField& f, const KumipuyoSeq& seq){
    // 無連鎖の時に少しだけ逃げれば先に相手が死ぬかも
    int y = INT_MAX;
    int fastestFrame = INT_MAX;
    Decision move;
    Plan::iterateAvailablePlans(f, seq, 2, [&](const RefPlan& plan) {
        if(plan.isRensaPlan()){
            //const RensaResult& rensa = plan.rensaResult();
            //const int chain = rensa.chains;
            //const int s = rensa.score;
            const int frame = plan.totalFrames();
            
            // 生き延びる(+嫌がらせ)のため最短で連鎖が起こるものを選ぶ
            if(frame < fastestFrame){
                move = plan.decision(0);
                fastestFrame = frame;
            }
        }else if(fastestFrame == INT_MAX && plan.field().height(3) < y){
            move = plan.decision(0);
            y = plan.field().height(3);
        }
    });
    
    return move;
}

class YuriCatAI : public AI {
    
public:
    YuriCatAI(int argc, char* argv[]) : AI(argc, argv, "miC") {
        
        win[0] = win[1] = 0;
        draw = 0;
        games = 0;
        
    }
    ~YuriCatAI() override {
        cerr << "DESTRUCTOR" << endl;
    }
    
    DropDecision think(int frameId, const CoreField& f, const KumipuyoSeq& seq,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const override{
        // 思考
        LOG(INFO) << f.toDebugString() << seq.toString();
        
        cerr << "turn " << turn[0] << endl;
        if(g_requestFrame[0].size() <= (unsigned int)turn[0]){
            g_requestFrame[0].push_back(frameId);
        }
        
        // 相手視点での探索を行っているかチェック
        if(g_searchThread.size() > 0){
            cerr << "should stop" << endl;
            // 相手視点での探索を終わらせる必要がある
            g_enemySearchNum = g_searchThread.size();
            while(g_enemySearchNum);
            cerr << "all stopped" << endl;
            //Decision enemy_best = g_soloSearchResults[1].bestFirst();
            //g_soloSearchResults[1].proceed(enemy_best);
            
            //cerr << enemy_best << endl;
            g_soloSearchResults[1].init();
            
            g_searchThread.clear();
        }
        
        // 現時点での連鎖力についてデバッグ
        {
            ChainResult best, second;
            detectMainChain(f, &best, &second);
            cerr << "my chain = " << get<0>(best) << endl;
            
            // 短手数で実際に起こせる連鎖があるか調べる
            for(int i = 0; i < 20; ++i){
                g_chainIgnitedRensaFrameScore[0][i].clear();
                g_chainTotalRensaFrameScore[0][i].clear();
            }
            g_ignitedRensaFrameScore[0].clear();
            g_totalRensaFrameScore[0].clear();
            
            int max_chain = 0;
            int max_score = 0;
            Plan::iterateAvailablePlans(f, seq, 2, [frameId, &max_chain, &max_score](const RefPlan& plan) {
                int ignited_frame = frameId + plan.framesToIgnite();
                int total_frame = frameId + plan.totalFrames();
                if(plan.isRensaPlan()){
                    const RensaResult& rensa = plan.rensaResult();
                    int chain = rensa.chains; // 1 ~ 19 のはず
                    int score = rensa.score;
                    
                    g_chainIgnitedRensaFrameScore[0][chain].push_back(make_tuple(ignited_frame, score));
                    g_chainTotalRensaFrameScore[0][chain].push_back(make_tuple(total_frame, score));
                    g_ignitedRensaFrameScore[0].push_back(make_tuple(ignited_frame, score));
                    g_totalRensaFrameScore[0].push_back(make_tuple(total_frame, score));
                    
                    max_chain = max(max_chain, chain);
                    max_score = max(max_score, score);
                }
            });
            g_minChain[0] = max_chain;
            g_minScore[0] = max_score;
            
            // 各連鎖数ごとのフレーム数を順番に並べる
            for(int i = 0; i < 20; ++i){
                sort(g_chainIgnitedRensaFrameScore[0][i].begin(), g_chainIgnitedRensaFrameScore[0][i].end());
                sort(g_chainTotalRensaFrameScore[0][i].begin(), g_chainTotalRensaFrameScore[0][i].end());
            }
            sort(g_ignitedRensaFrameScore[0].begin(), g_ignitedRensaFrameScore[0].end());
            sort(g_totalRensaFrameScore[0].begin(), g_totalRensaFrameScore[0].end());
            
            cerr << "my real score = " << g_minScore[0] << endl;
        }
        
        cerr << enemy.currentChain << endl;
        /*if(enemy.isRensaOngoing()){
            //g_oppMinChain = max(g_oppMinChain, enemy.currentChain);
            g_minScore[1] = max(g_minScore[1], scoreForOjama(me.totalOjama(enemy)));
        }*/
        if(enemy.isRensaOngoing()){
            g_minScore[1] = max(g_minScore[1], scoreForOjama(me.totalOjama(enemy)));
        }else{
            g_minScore[1] = g_minScore[1] + scoreForOjama(me.totalOjama(enemy));
        }
        // cerr << "enemy real chain = " << g_oppMinChain << endl;
        cerr << "enemy real score = " << g_minScore[1] << endl;
        
        // DCHECK(!fast);
        
        Decision best; // 自動で(0, 0)初期化
        std::string bestMessage;
        
        //cerr << "sequence = " << sequence.toString() << endl;
        
        CERR << f;
        
        g_limitTime = ClockMS::now() + timeManage(me, enemy, fast);
        cerr << g_limitTime << endl;
        
        // 定石
        // 3連鎖以下の全消しがあれば即打ち
        // 全消し後に1連鎖があれば即打ち
        // 高確率のMATEがあれば即打ち(バグっているので外した)
        Decision mate_move;
        double mate_rate = 0;
        
        Decision zenkeshi_move;
        int zenkeshi_score = -SCORE_MATE;
        
        Decision after_zenkeshi_move;
        int after_zenkeshi_score = 999999;
        
        //int rensa_finish_frame = enemy.isRensaOngoing() ? enemy.rensaFinishingFrameId() : INT_MAX;
        
        Plan::iterateAvailablePlans(f, seq, 2, [&](const RefPlan& plan) {
            if(plan.isRensaPlan()){
                const RensaResult& rensa = plan.rensaResult();
                const int chain = rensa.chains;
                const int s = rensa.score;
                
                // mate check
                /*double mr = isOjamaMate(enemy.fieldWhenGrounded, enemy.seq, g_requestFrame[1][turn[1]], frameId + plan.totalFrames(), s);
                if(mr > 0.6){
                    mate_rate = mr;
                    mate_move = plan.decision(0);
                }*/
                
                // zenkeshi check
                if(chain <= 3){
                    if(plan.field().countPuyos() == 0){
                        if (s > zenkeshi_score) {
                            zenkeshi_score = s;
                            zenkeshi_move = plan.decision(0);
                        }
                    }
                }
                
                // after_zenkeshi check
                if(me.hasZenkeshi){
                    if(s < after_zenkeshi_score){
                        after_zenkeshi_score = s;
                        after_zenkeshi_move = plan.decision(0);
                    }
                }
            }
        });
        
        
        if(mate_rate > 0){
            cerr << "mate_rate = " << mate_rate << endl;
            return DropDecision(mate_move, "Mate");
        }
        
        if(zenkeshi_score > 0){
            cerr << "zenkeshi score = " << zenkeshi_score << endl;
            return DropDecision(zenkeshi_move, "Zenkeshi");
        }
        
        if(after_zenkeshi_score < 1000){
            cerr  << "after zenkeshi score = " << after_zenkeshi_score << endl;
            return DropDecision(after_zenkeshi_move, "AfterZenkeshi");
        }
        
        if(evaluationMode){
            int score = -SCORE_MATE;
            int rensa_finish_frame = enemy.isRensaOngoing() ? enemy.rensaFinishingFrameId() : INT_MAX;
            Plan::iterateAvailablePlans(f, seq, 2, [frameId, f, rensa_finish_frame, &best, &score](const RefPlan& plan) {
                int frame = frameId + plan.totalFrames();
                if(frame < rensa_finish_frame){
                    int s = evaluatePlan(f, plan);
                    
                    if (s > score) {
                        score = s;
                        best = plan.decision(0);
                        CERR << plan.decision(0).toString() << ", " << plan.decision(1).toString() << endl;
                    }
                }
            });
            
            cerr << "best move = " << best.toString() << " plan score = " << score << endl;
        }else{
            //ASSERT(sequence.get(turn[0]) == seq.get(0), cerr << sequence.toString() << " " << seq.toString() << " " << turn[0] << endl;);
            //sleep(0.3);
            g_searchThread.clear();
            
            for (int i = 0; i < num_threads; ++i){
                g_searchThread.emplace_back(&soloSimulateThread,
                                            i, 0, me, enemy, seq, turn[0], frameId);
            }
            for (auto& th : g_searchThread){
                th.join();
            }
            
            DropDecision bestDM = g_soloSearchResults[0].bestFirst();
            best = bestDM.decision();
            bestMessage = bestDM.message();
            g_soloSearchResults[0].proceed(best);
            
            // 相手視点での探索スタート
            g_searchThread.clear();
            /*for (int i = 0; i < num_threads - 1; ++i){  // 1スレッド余らせる
                g_searchThread.emplace_back(&soloSimulateThread,
                                            i, 1, enemy, me, enemy.seq, turn[1], frameId);
            }
            for (auto& th : g_searchThread){
                th.detach();
            }*/
        }
        
        if(best == Decision()){
            return DropDecision(searchEscapeMove(f, seq), "Give Up");
        }
        return DropDecision(best, bestMessage);
    }
        
    void gaze(int frameId, const CoreField& enemyField, const KumipuyoSeq& seq) override{
        // 相手状態の取得
        UNUSED_VARIABLE(frameId);
        UNUSED_VARIABLE(seq);
        
        // 相手の連鎖力を調べる
        ChainResult best, second;
        detectMainChain(enemyField, &best, &second);
        cerr << "enemy chain = " << get<0>(best) << endl;
        
        // 短手数で実際に起こせる連鎖があるか調べる
        for(int i = 0; i < 20; ++i){
            g_chainIgnitedRensaFrameScore[1][i].clear();
            g_chainTotalRensaFrameScore[1][i].clear();
        }
        g_ignitedRensaFrameScore[1].clear();
        g_totalRensaFrameScore[1].clear();
        
        int max_chain = 0;
        int max_score = 0;
        Plan::iterateAvailablePlans(enemyField, seq, 2, [frameId, enemyField, &max_chain, &max_score](const RefPlan& plan) {
            int ignited_frame = frameId + plan.framesToIgnite();
            int total_frame = frameId + plan.totalFrames();
            if(plan.isRensaPlan()){
                const RensaResult& rensa = plan.rensaResult();
                int chain = rensa.chains; // 1 ~ 19 のはず
                int score = rensa.score;
                
                g_chainIgnitedRensaFrameScore[1][chain].push_back(make_tuple(ignited_frame, score));
                g_chainTotalRensaFrameScore[1][chain].push_back(make_tuple(total_frame, score));
                g_ignitedRensaFrameScore[1].push_back(make_tuple(ignited_frame, score));
                g_totalRensaFrameScore[1].push_back(make_tuple(total_frame, score));
                
                max_chain = max(max_chain, chain);
                max_score = max(max_score, score);
            }
        });
        g_minChain[1] = max_chain;
        g_minScore[1] = max_score;
        
        // 各連鎖数ごとのフレーム数を順番に並べる
        for(int i = 0; i < 20; ++i){
            sort(g_chainIgnitedRensaFrameScore[1][i].begin(), g_chainIgnitedRensaFrameScore[1][i].end());
            sort(g_chainTotalRensaFrameScore[1][i].begin(), g_chainTotalRensaFrameScore[1][i].end());
        }
        sort(g_ignitedRensaFrameScore[1].begin(), g_ignitedRensaFrameScore[1].end());
        sort(g_totalRensaFrameScore[1].begin(), g_totalRensaFrameScore[1].end());
        
    }
    
    // 情報アップデート系
    void onGameWillBegin(const FrameRequest& request) override{ // 1試合開始時
        UNUSED_VARIABLE(request);
        
        // 相手視点での探索を行っているかチェック
        if(g_searchThread.size() > 0){
            cerr << "should stop in gs" << endl;
            // 相手視点での探索を終わらせる必要がある
            g_enemySearchNum = g_searchThread.size();
            while(g_enemySearchNum);
            cerr << "all stopped in gs" << endl;
            g_searchThread.clear();
        }
        
        for(int i = 0; i < 2; ++i){
            turn[i] = isEndlessMode() ? -1 : 0; // endless_modeではthinkの前に加算されるため
            seq_num[i] = 0;
            
            g_requestFrame[i].clear();
            g_soloSearchResults[i].init();
        }
        
        sequence.clear();
        
        g_enemySearchNum = 0;
        
        cerr << "game " << games << ": frame = " << request.frameId << endl;
        
        // 全スレッドを起動して思考開始
        /*for(int t = 0; t < num_threads; ++t){
            thread<soloSimulation<>>
        }*/
    }
    void onGameHasEnded(const FrameRequest& request) override{ // 1試合終了時
        switch(request.gameResult){
            case GameResult::P1_WIN: win[0] += 1; break;
            case GameResult::P2_WIN: win[1] += 1; break;
            default: draw += 1;
        }
        games += 1;
    }
    
    void onPreDecisionRequestedForMe(const FrameRequest& request) override{
        UNUSED_VARIABLE(request);
    }
    void onDecisionRequestedForMe(const FrameRequest& request) override{
        ++turn[0];
        if(isEndlessMode() && turn[0] == 0){ // 試合開始時の処理 next1, 2が確定
            sequence.add(request.myPlayerFrameRequest().kumipuyoSeq.get(0));
            sequence.add(request.myPlayerFrameRequest().kumipuyoSeq.get(1));
            seq_num[0] = seq_num[1] = 2;
        }
    }
    void onGroundedForMe(const FrameRequest& request) override{
        UNUSED_VARIABLE(request);
    }
    void onPuyoErasedForMe(const FrameRequest& request) override{
        UNUSED_VARIABLE(request);
    }
    void onOjamaDroppedForMe(const FrameRequest& request) override{
        UNUSED_VARIABLE(request);
    }
    void onNext2AppearedForMe(const FrameRequest& request) override{
        if((!isEndlessMode() || seq_num[0] >= 2) && seq_num[0] >= seq_num[1]){
            sequence.add(request.myPlayerFrameRequest().kumipuyoSeq.get(1));
        }
        ++seq_num[0];
    }
        
    void onDecisionRequestedForEnemy(const FrameRequest& request) override{
        UNUSED_VARIABLE(request);
        ++turn[1];
        g_requestFrame[1].push_back(request.frameId);
    }
    void onGroundedForEnemy(const FrameRequest& request) override{
        UNUSED_VARIABLE(request);
    }
    void onPuyoErasedForEnemy(const FrameRequest& request) override{
        UNUSED_VARIABLE(request);
    }
    void onOjamaDroppedForEnemy(const FrameRequest& request) override{
        UNUSED_VARIABLE(request);
    }
    void onNext2AppearedForEnemy(const FrameRequest& request) override{
        if((!isEndlessMode() || seq_num[1] >= 2) && seq_num[1] >= seq_num[0]){
            sequence.add(request.enemyPlayerFrameRequest().kumipuyoSeq.get(1));
        }
        ++seq_num[1];
    }
        
    //void setEndlessMode(){
    //    endless_mode = true;
    //}
        
    private:
    
    void addKumipuyo(const Kumipuyo& kumipuyo){
        sequence.add(kumipuyo);
        g_soloSearchResults[0].updateSequence(max(seq_num[0], seq_num[1]), kumipuyo);
        g_soloSearchResults[1].updateSequence(max(seq_num[0], seq_num[1]), kumipuyo);
    }

    KumipuyoSeq sequence; // ゲーム開始時からのKumipuyoを記録
    int seq_num[2];
    int turn[2]; // 双方の思考回数
    int win[2], draw; // 勝敗
    int games;
    //int rensa_finish_frame[2]; // 連鎖終了フレーム
    bool evaluationMode = false; // 評価関数1手読みモード
    int num_threads = 3;
    //bool endless_mode = false; // 諸事情によりendless modeか知る必要がある
};
        
int soloLoop(const int loops, const int seed){
    // 一人で本線を打ち終わるまで
    
    std::mt19937 mt(seed);
    valarray<int> chain(loops);
    valarray<int> score(loops);
    
    for(int l = 0; l < loops; ++l){
        std::unique_ptr<AI> ai(new YuriCatAI({}, 0));
        setEndlessMode();
        Endless endless(std::move(ai));
        KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2SequenceWithSeed(mt());
        EndlessResult result = endless.run(seq);
        chain[l] = result.maxRensa;
        score[l] = result.score;
        cerr << "trial " << l << ": chain = " << result.maxRensa << " score = " << result.score << endl;
    }
    cerr << "avg chain = " << (chain.sum() / (float)loops) << endl;
    cerr << "avg score = " << (score.sum() / (float)loops) << endl;
    return 0;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif
    
    //setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    
    srand((unsigned int)time(NULL));
    
    if (FLAGS_loop == 0){
        YuriCatAI(argc, argv).runLoop();
    }else{
        soloLoop(FLAGS_loop, FLAGS_random_seed);
    }
    return 0;
}
