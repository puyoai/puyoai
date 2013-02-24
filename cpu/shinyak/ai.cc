#include "ai.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <glog/logging.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "core/constant.h"
#include "core/decision.h"
#include "ctrl.h"
#include "drop_decision.h"
#include "evaluation_feature.h"
#include "evaluation_feature_collector.h"
#include "game.h"
#include "puyo.h"
#include "puyo_possibility.h"
#include "rensa_detector.h"
#include "rensa_info.h"
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
    if (dropDecision.decision().IsValid())
        m_myPlayerInfo.puyoDropped(dropDecision.decision(), game.myPlayerState().kumiPuyos[0]);
}

void AI::wnextAppeared(const Game& game)
{
    // Since the current puyo is moving
    vector<KumiPuyo> kumiPuyos(game.myPlayerState().kumiPuyos.begin() + 1,
                               game.myPlayerState().kumiPuyos.end());
    m_myPlayerInfo.updateMainRensa(kumiPuyos);
}

void AI::myRensaFinished(const Game& game)
{
    m_myPlayerInfo.rensaFinished(game.myPlayerState().field);

    vector<KumiPuyo> kumiPuyos(game.myPlayerState().kumiPuyos.begin(),
                               game.myPlayerState().kumiPuyos.begin() + 2);
    m_myPlayerInfo.updateMainRensa(kumiPuyos);
}

void AI::myOjamaDropped(const Game& game)
{
    m_myPlayerInfo.ojamaDropped(game.myPlayerState().field);

    vector<KumiPuyo> kumiPuyos(game.myPlayerState().kumiPuyos.begin(),
                               game.myPlayerState().kumiPuyos.begin() + 2);
    m_myPlayerInfo.updateMainRensa(kumiPuyos);
}

void AI::enemyWNextAppeared(const Game& game)
{
    int currentFrameId = game.id;

    m_enemyInfo.setId(currentFrameId);

    m_enemyInfo.updateFeasibleRensas(game.enemyPlayerState().field, game.enemyPlayerState().kumiPuyos);
    m_enemyInfo.updatePossibleRensas(game.enemyPlayerState().field, game.enemyPlayerState().kumiPuyos);

    LOG(INFO) << "Possible rensa infos : ";
    for (auto it = m_enemyInfo.possibleRensaInfos().begin(); it != m_enemyInfo.possibleRensaInfos().end(); ++it)
        LOG(INFO) << it->toString();
    LOG(INFO) << "Feasible rensa infos : ";
    for (auto it = m_enemyInfo.feasibleRensaInfos().begin(); it != m_enemyInfo.feasibleRensaInfos().end(); ++it)
        LOG(INFO) << it->toString();
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
    LOG(INFO) << "AI::decide is called";
    LOG(INFO) << m_myPlayerInfo.estimatedField().getDebugOutput();

    const int depth = 2;
    std::vector<Plan> plans;
    findAvailablePlans(m_myPlayerInfo.estimatedField(), depth, game.myPlayerState().kumiPuyos, plans);

    LOG(INFO) << "FindAvailablePlans: OK";
    
    double currentBestScore = -100.0;
    for (std::vector<Plan>::iterator it = plans.begin(); it != plans.end(); ++it) {
        EvalResult result = eval(game.id, *it, game.myPlayerState().field);
        if (currentBestScore < result.evaluationScore) {
            currentBestScore = result.evaluationScore;
            dropDecision = DropDecision(it->firstHandDecision(), result.message);
        }
    }
}

EvalResult AI::eval(int currentFrameId, const Plan& plan, const Field& currentField) const
{
    // TODO: AI は全消しを理解するべき

    if (m_enemyInfo.rensaIsOngoing() && m_enemyInfo.ongoingRensaInfo().rensaInfo.score > scoreForOjama(6)) {
        // If we have a firable plan before the enemy Rensa has been finished, we would like to
        // use it for parry (TAIOH).

        // TODO: 受けるか受けないかを判定する。受ける場合を実装したら if 文から > 100 を消す。

        // TODO: 対応が適当すぎる
        if (m_enemyInfo.ongoingRensaInfo().rensaInfo.score >= scoreForOjama(6) &&
            plan.totalScore() >= m_enemyInfo.ongoingRensaInfo().rensaInfo.score &&
            plan.initiatingFrames() <= m_enemyInfo.ongoingRensaInfo().finishingRensaFrame) {
            LOG(INFO) << plan.decisionText() << " TAIOU";
            return EvalResult(70.0 + plan.totalScore() / 1000000.0, "TAIOU");
        }

        // TODO: ちょっとぐらい負けていても、食らうぐらいなら対応を打ったほうが良い場合がある

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
            LOG(INFO) << plan.decisionText() << " ZENKESHI";
            return EvalResult(90.0 + 1.0 / plan.totalFrames(), "ZENKESHI");
        }
        
        int estimatedMaxScore = m_enemyInfo.estimateMaxScore(rensaEndingFrameId);
        // log << "ESTIMATED MAX SCORE = " << estimatedMaxScore << " BY " << rensaEndingFrameId << endl;

        // --- 1.1. 十分でかい場合は打って良い。
        // / TODO: 十分でかいとは？ / とりあえず致死量ということにする
        if (plan.totalScore() >= estimatedMaxScore + scoreForOjama(60)) {
            ostringstream ss;
            ss << "LARGE ENOUGH : my score=" << plan.totalScore() << " enemy score=" << estimatedMaxScore << " rensa ends=" << rensaEndingFrameId;
            LOG(INFO) << plan.decisionText() << " " << ss.str();
            return EvalResult(100.0 + 1.0 / plan.totalFrames(), ss.str());
        }
        
        // --- 1.2. 対応手なく潰せる
        // TODO: 実装があやしい。
        if (plan.totalScore() >= scoreForOjama(18)) {
            if (estimatedMaxScore <= scoreForOjama(6)) {
                ostringstream ss;
                ss << "TSUBUSHI " << plan.totalScore() << " " << estimatedMaxScore << endl;
                LOG(INFO) << plan.decisionText() << " " << ss.str();
                return EvalResult(70.0 + 1.0 / plan.totalFrames(), ss.str());
            }
        }
        
        // --- 1.3. 飽和したので打つしかなくなった
        // TODO: これは EnemyRensaInfo だけじゃなくて MyRensaInfo も必要なのでは……。
        // TODO: 60 個超えたら打つとかなんか間違ってるだろう。
        if (currentField.countPuyos() >= 56) {
            ostringstream ss;
            ss << "HOUWA " << plan.totalScore();
            LOG(INFO) << plan.decisionText() << ss.str();
            return EvalResult(60.0 + plan.totalScore() / 1000000.0, ss.str());
        }
        
        // --- 1.4. 打つと有利になる
        // TODO: そもそも数値化の仕方が分からない。
        
        // 基本的に先打ちすると負けるので、打たないようにする
        ostringstream ss;
        ss << "SAKIUCHI will lose : score = " << plan.totalScore() << " EMEMY score = " << estimatedMaxScore << endl;
        LOG(INFO) << plan.decisionText() << " " << ss.str();
        return EvalResult(-1.0, ss.str());
    }

    EvaluationFeature feature;

    EvaluationFeatureCollector::collectEmptyAvailabilityFeature(feature, plan.field());

    {
        int maxChains = 0;
        int numNecessaryPuyos = 0;

        vector<PossibleRensaInfo> rensaInfos;
        RensaDetector::findPossibleRensas(rensaInfos, plan.field());
        for (vector<PossibleRensaInfo>::iterator it = rensaInfos.begin(); it != rensaInfos.end(); ++it) {
            if (maxChains < it->rensaInfo.chains) {
                maxChains = it->rensaInfo.chains;
                numNecessaryPuyos = TsumoPossibility::necessaryPuyos(0.5, it->necessaryPuyoSet);
            } else if (maxChains == it->rensaInfo.chains) {
                numNecessaryPuyos = min(numNecessaryPuyos, TsumoPossibility::necessaryPuyos(0.5, it->necessaryPuyoSet));
            }
        }

        feature.set(MAX_CHAINS, maxChains);
        feature.set(MAX_RENSA_NECESSARY_PUYOS, numNecessaryPuyos);
    }

    EvaluationFeatureCollector::collectConnectionFeature(feature, plan.field(), m_myPlayerInfo.mainRensaTrackResult());
    EvaluationFeatureCollector::collectFieldHeightFeature(feature, plan.field());
    feature.set(TOTAL_FRAMES, plan.totalFrames());
    double handWidth = 3 * m_myPlayerInfo.mainRensaHandWidth();
    if (handWidth <= 0)
        handWidth = -3;

    double finalScore = 
        + handWidth
        + feature.calculateScore();
    
    char buf[256];
    sprintf(buf, "eval-score: %.3f : = %.3f : maxChain = %d : enemy = %d : %d : %d ",
            handWidth,
            finalScore,
            m_myPlayerInfo.mainRensaChains(),
            m_enemyInfo.estimateMaxScore(currentFrameId + plan.totalFrames()),
            m_enemyInfo.estimateMaxScore(currentFrameId + plan.totalFrames() + 50),
            m_enemyInfo.estimateMaxScore(currentFrameId + plan.totalFrames() + 100));

    LOG(INFO) << plan.decisionText() << " Extending HONSEN : " << buf;    
    return EvalResult(finalScore, buf);
}


