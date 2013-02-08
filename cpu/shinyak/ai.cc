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

EvalResult::EvalResult(double evaluation, const string& message)
    : evaluationScore(evaluation)
    , message(message)
{
    for (string::size_type i = 0; i < this->message.size(); ++i) {
        if (this->message[i] == ' ')
            this->message[i] = '_';
    }
}

std::string AI::getName() const
{
    return "shinyak";
}

AI::AI(const string& name)
    : m_name(name)
    , log((name + ".txt").c_str())
{
}

void AI::initialize(const Game& game)
{
    m_enemyInfo.initializeWith(game.id);
    m_myPlayerInfo.initialize();
    m_myPlayerInfo.forceEstimatedField(game.myPlayerState().field);
}

void AI::think(DropDecision& dropDecision, const Game& game)
{
    decide(dropDecision, game);
    if (dropDecision.decision().isValid())
        m_myPlayerInfo.puyoDropped(dropDecision.decision(), game.myPlayerState().kumiPuyos[0]);
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
    field.forceDrop();

    BasicRensaInfo rensaInfo;
    field.simulate(rensaInfo);
    
    if (rensaInfo.chains > 0)
        m_enemyInfo.setOngoingRensa(OngoingRensaInfo(rensaInfo, game.id + rensaInfo.frames));
    else
        m_enemyInfo.setRensaIsOngoing(false);
}

void AI::decide(DropDecision& dropDecision, const Game& game)
{
    log << "AI::decide is called" << endl;
    log << m_myPlayerInfo.estimatedField().getDebugOutput() << endl;

    const int depth = 2;
    std::vector<Plan> plans;
    m_myPlayerInfo.estimatedField().findAvailablePlans(depth, game.myPlayerState().kumiPuyos, plans);

    log << "FindAvailablePlans: OK" << endl;
    
    double currentBestScore = -100.0;
    for (std::vector<Plan>::iterator it = plans.begin(); it != plans.end(); ++it) {
        EvalResult result = eval(game.id, *it);
        if (currentBestScore < result.evaluationScore) {
            currentBestScore = result.evaluationScore;
            dropDecision = DropDecision(it->firstHandDecision(), result.message);
        }
    }
}

EvalResult AI::eval(int currentFrameId, const Plan& plan) const
{
    for (int i = 0; i < plan.numDecisions(); ++i)
        log << plan.decision(i).toString();
    log << ' ';

    if (m_enemyInfo.rensaIsOngoing() && m_enemyInfo.ongoingRensaInfo().rensaInfo.score > scoreForOjama(6)) {
        // If we have a firable plan before the enemy Rensa has been finished, we would like to
        // use it for parry (TAIOH).

        // TODO: 受けるか受けないかを判定する。受ける場合を実装したら if 文から > 100 を消す。

        // TODO: 対応が適当すぎる
        if (m_enemyInfo.ongoingRensaInfo().rensaInfo.score >= scoreForOjama(6) &&
            plan.totalScore() >= m_enemyInfo.ongoingRensaInfo().rensaInfo.score &&
            plan.initiatingFrames() <= m_enemyInfo.ongoingRensaInfo().finishingRensaFrame) {
            return EvalResult(70.0 + plan.totalScore() / 1000000.0, "TAIOU");
        }

        // TODO: 割とどうしようもない場合に高く積むというルーチンを持つべき
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
            return EvalResult(90.0 + 1.0 / plan.totalFrames(), "ZENKESHI");
        }
        
        int estimatedMaxScore = m_enemyInfo.estimateMaxScore(rensaEndingFrameId);
        log << "ESTIMATED MAX SCORE = " << estimatedMaxScore << " BY " << rensaEndingFrameId << endl;

        // --- 1.1. 十分でかい場合は打って良い。
        // / TODO: 十分でかいとは？ / とりあえず致死量ということにする
        if (plan.totalScore() >= estimatedMaxScore + scoreForOjama(60)) {
            ostringstream ss;
            ss << "LARGE ENOUGH : " << plan.totalScore() << " " << estimatedMaxScore;
            return EvalResult(100.0 + 1.0 / plan.totalFrames(), ss.str());
        }
        
        // --- 1.2. 対応手なく潰せる
        // TODO: 実装があやしい。
        if (plan.totalScore() >= scoreForOjama(18)) {
            if (estimatedMaxScore <= scoreForOjama(6)) {
                ostringstream ss;
                ss << "TSUBUSHI " << plan.totalScore() << " " << estimatedMaxScore << endl;
                return EvalResult(70.0 + 1.0 / plan.totalFrames(), ss.str());
            }
        }
        
        // --- 1.3. 飽和したので打つしかなくなった
        // TODO: これは EnemyRensaInfo だけじゃなくて MyRensaInfo も必要なのでは……。
        // TODO: 60 個超えたら打つとかなんか間違ってるだろう。
        if (plan.field().countPuyos() >= 60)
            return EvalResult(60.0 + plan.totalScore() / 1000000.0, "HOUWA");
        
        // --- 1.4. 打つと有利になる
        // TODO: そもそも数値化の仕方が分からない。
        
        // 基本的に先打ちすると負けるので、打たないようにする
        log << "SAKIUCHI will lose : score = " << plan.totalScore()
            << " EMEMY score = " << estimatedMaxScore << endl;

        return EvalResult(-1.0, "SAKIUCHI will lose");
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

    double finalScore = 
        + emptyFieldAvailability / (78 - colorPuyoNum)
        + maxChains
        + fieldScore / 30
        + fieldHeightScore
        + frameScore;
    
    char buf[80];
    sprintf(buf, "eval-score: %f %d %f %f %f : = %f",
            emptyFieldAvailability / (78 - colorPuyoNum),
            maxChains,
            fieldScore / 30,
            fieldHeightScore,
            frameScore,
            finalScore);
    log << buf << endl;
    
    return EvalResult(finalScore, "extending HONSEN");
}


