#include "ai.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <glog/logging.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../../core/constant.h"
#include "ctrl.h"
#include "decision.h"
#include "field_evaluator.h"
#include "game.h"
#include "puyo.h"
#include "rensainfo.h"
#include "score.h"

using namespace std;

std::string AI::getName() const
{
    return "shinyak";
}

void AI::initialize(const Game& game)
{
    m_enemyInfo.initializeWith(game.id);
    m_myPlayerInfo.initialize();
}

void AI::think(Decision& decision, const Game& game, std::ofstream& log)
{
    decide(game, &decision, log);
    if (decision.isValid())
        m_myPlayerInfo.puyoDropped(decision, game.myPlayerState().kumiPuyos[0]);
}

void AI::myRensaFinished(const Game& game)
{
    m_myPlayerInfo.rensaFinished(game.myPlayerState().field);
}

void AI::myOjamaDropped(const Game& game)
{
    m_myPlayerInfo.ojamaDropped(game.myPlayerState().field);
}

void AI::enemyWNextAppeared(const Game& game)
{
    int currentFrameId = game.id;

    m_enemyInfo.setId(currentFrameId);
    m_enemyInfo.setEmptyFieldAvailability(FieldEvaluator::calculateEmptyFieldAvailability(game.enemyPlayerState().field));

    m_enemyInfo.updateFeasibleRensas(game.enemyPlayerState().field, game.enemyPlayerState().kumiPuyos);
    m_enemyInfo.updatePossibleRensas(game.enemyPlayerState().field, game.enemyPlayerState().kumiPuyos);
}

void AI::enemyGrounded(const Game& game)
{
    m_enemyInfo.setId(game.id);

    // --- Check if Rensa starts.
    Field field(game.enemyPlayerState().field);

    BasicRensaInfo rensaInfo;
    field.simulate(rensaInfo);
    
    if (rensaInfo.chains > 0)
        m_enemyInfo.setOngoingRensa(OngoingRensaInfo(rensaInfo, game.id + rensaInfo.frames));
    else
        m_enemyInfo.setRensaIsOngoing(false);
}

void AI::decide(const Game& game, Decision* decision, std::ofstream& log)
{
    log << "AI::decide is called" << endl;
    log << m_myPlayerInfo.estimatedField().getDebugOutput() << endl;

    const int depth = 2;
    std::vector<Plan> plans;
    m_myPlayerInfo.estimatedField().findAvailablePlans(depth, game.myPlayerState().kumiPuyos, plans);

    log << "FindAvailablePlans: OK" << endl;
    
    double currentMaxPlanScore = -100.0;
    for (std::vector<Plan>::iterator it = plans.begin(); it != plans.end(); ++it) {
        double planScore = eval(game.id, *it, log);
        if (currentMaxPlanScore < planScore) {
            currentMaxPlanScore = planScore;
            *decision = it->firstHandDecision();
        }
    }
}

// 評価関数の案
// 1. 各セルに空間効率を定義し、その効率の和を最大化する？　効率の和で飽和連鎖量を定義できそう
//    空きマスは 0.8 ぐらいの効率？
//    連結しているぷよの方が効率が高い
//    呼吸点から遠いぷよは効率が悪い
//    　呼吸点には連鎖前呼吸点と連鎖後呼吸点が存在
//　　このあたりを使って
// 2. くぼみを作らないようにする
//    適当にくぼみを作ると評価値を下げるとかよりも
//    あるマスから上に向かってスポットライト的に上３マスほどを見た場合に、空間が多いほど効率が良いことになる
//    　端に窪みが出来るのはこの種類の評価で結構防げるはず
//    結局空きマスの効率をうまく定義しようとする試みに近い

// eval() should return [-1, 1]
double AI::eval(int currentFrameId, const Plan& plan, std::ofstream& log) const
{
    for (int i = 0; i < plan.numDecisions(); ++i)
        log << plan.decision(i).toString();
    log << ' ';

    if (m_enemyInfo.rensaIsOngoing() && m_enemyInfo.ongoingRensaInfo().rensaInfo.score > 100) {
        // If we have a firable plan before the enemy Rensa has been finished, we would like to
        // use it for parry (TAIOH).

        // 受ける場合と受けない場合を考えたい
        // 割とどうしようもない場合に高く積むというルーチンとかどうするべきか

        if (m_enemyInfo.ongoingRensaInfo().rensaInfo.score >= scoreForOjama(6) &&
            plan.totalScore() >= m_enemyInfo.ongoingRensaInfo().rensaInfo.score &&
            plan.initiatingFrames() <= m_enemyInfo.ongoingRensaInfo().finishingRensaFrame) {
            log << "TAIOU" << endl;
            return 70.0 + plan.totalScore() / 1000000.0;
        }
    }

    if (plan.isRensaPlan()) {
        int rensaEndingFrameId = currentFrameId + plan.totalFrames();
        // このプランを選ぶことは、連鎖を先打ちすることになる
        // 連鎖を先打ちしていいのは次のような場合：
        // 1. こちらから勝てると確信して連鎖を打つ場合
        //    1.1. この plan が相手より十分でかいことが確信できるなら、打つ
        //    1.2. この plan で相手を潰せるなら、打つ
        //    1.3. この plan で飽和しているなら、どうしようもないので打つ
        // 2. 勝てる確証はないが有利に立てそうと判断して打つ場合
        //    2.1. 相手の対応後に数的有利に立てそうであれば、打つ（催促）
        
        // --- 1.0. 全消しは、とりあえず取る(TODO: よくない)
        if (plan.field().isZenkeshi()) {
            log << "ZENKESHI: frame = " << plan.totalFrames() << endl;
            return 90.0 + 1.0 / plan.totalFrames();
        }
        
        int estimatedMaxScore = m_enemyInfo.estimateMaxScore(rensaEndingFrameId);
        log << "ESTIMATED MAX SCORE = " << estimatedMaxScore << " BY " << rensaEndingFrameId << endl;

        // --- 1.1. 十分でかい / TODO: 十分でかいとは？ / とりあえず致死量ということにする
        if (plan.totalScore() >= estimatedMaxScore + scoreForOjama(72)) {
            log << "large enough: score = 100" << endl;
            return 100.0;
        }
        
        // --- 1.2. 対応手なく潰せる
        if (plan.totalScore() >= scoreForOjama(18)) {
            if (estimatedMaxScore <= scoreForOjama(6)) {
                // TODO: これはちょっと怪しい……
                log << "TSUBUSHI: score = 70; enemy score = " << estimatedMaxScore << endl;
                return 70.0;
            }
        }
        
        // --- 1.3. 飽和したので打つしかなくなった
        // ちょっとわからんので放置
        // TODO: これは EnemyRensaInfo だけじゃなくて MyRensaInfo も必要なのでは……。
        if (plan.field().countPuyos() >= 60)
            return 60.0 + plan.totalScore() / 1000000.0;
        
        // --- 2.1. 有利に立てそう
        // これはどう数値化するべきか……
        
        // 基本的に先打ちすると負けるので、打たないようにする
        log << "SAKIUCHI will lose : score = " << plan.totalScore()
            << " EMEMY score = " << estimatedMaxScore << endl;

        return -0.6;
    }
    
    // 打たない場合、こちらの手を伸ばすことになるが、どのように伸ばすかが難しい。
    // というか、自分もどう伸ばしているのか、うまく言語化できない。
    // - 相手の本線よりもこちらが劣っていれば、本線を優先する
    // - 相手の副砲
    
    // とりあえず最初は適当な評価関数でいいか……。
    // emptyAvailability , fieldAvailability, 連鎖スコアを
    // 最も長い連鎖をうったときに残った形の連結の具合を評価関数にしてみるか……

    double emptyFieldAvailability = FieldEvaluator::calculateEmptyFieldAvailability(plan.field());

    int maxChains = 0;
    vector<PossibleRensaInfo> rensaInfos;
    plan.field().findRensas(rensaInfos);
    for (vector<PossibleRensaInfo>::iterator it = rensaInfos.begin(); it != rensaInfos.end(); ++it)
        maxChains = std::max(it->rensaInfo.chains, maxChains);

    int colorPuyoNum = plan.field().countColorPuyos();
    double fieldScore = FieldEvaluator::calculateConnectionScore(plan.field());
    double fieldHeightScore = FieldEvaluator::calculateFieldHeightScore(plan.field());
    double frameScore = 1.0 / plan.totalFrames();
    if (plan.totalFrames() >= 55)
        frameScore = 0;

    double finalScore = emptyFieldAvailability / (78 - colorPuyoNum)
        + maxChains
        + fieldScore / 10
        + fieldHeightScore
        + frameScore;
    
    char buf[80];
    sprintf(buf, "eval-score: %d %f %d %f",
            plan.totalFrames(),
            emptyFieldAvailability / (78 - colorPuyoNum),
            maxChains,
            finalScore);
    log << buf << endl;
    
    return finalScore / 10;
}


