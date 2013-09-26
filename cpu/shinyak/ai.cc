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
#include "core/score.h"
#include "ctrl.h"
#include "drop_decision.h"
#include "evaluation_feature.h"
#include "evaluation_feature_collector.h"
#include "game.h"
#include "puyo.h"
#include "puyo_possibility.h"
#include "rensa_detector.h"
#include "rensa_info.h"

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
    , m_evaluationParams("feature.txt")
{
    LOG(INFO) << m_evaluationParams.toString();
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
}

void AI::myRensaFinished(const Game& game)
{
    m_myPlayerInfo.rensaFinished(game.myPlayerState().field);

    vector<KumiPuyo> kumiPuyos(game.myPlayerState().kumiPuyos.begin(),
                               game.myPlayerState().kumiPuyos.begin() + 2);
}

void AI::myOjamaDropped(const Game& game)
{
    m_myPlayerInfo.ojamaDropped(game.myPlayerState().field);

    vector<KumiPuyo> kumiPuyos(game.myPlayerState().kumiPuyos.begin(),
                               game.myPlayerState().kumiPuyos.begin() + 2);
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
        EvalResult result = eval(game.id, *it);
        if (currentBestScore < result.evaluationScore) {
            currentBestScore = result.evaluationScore;
            dropDecision = DropDecision(it->firstHandDecision(), result.message);
        }
    }

    LOG(INFO) << "Decided : " << dropDecision.decision().toString();
}

EvalResult AI::eval(int currentFrameId, const Plan& plan) const
{
    EvaluationFeature feature;
    EvaluationFeatureCollector::collectFeatures(feature, plan, NUM_KEY_PUYOS, currentFrameId, m_enemyInfo);

    const RensaEvaluationFeature& bestRensaFeature = feature.findBestRensaFeature(m_evaluationParams);
    double finalScore = feature.calculateScoreWith(m_evaluationParams, bestRensaFeature);

    char buf[256];
    sprintf(buf, "max rensa = %d : eval score = %f : enemy = %d : %d : %d",
            bestRensaFeature.get(MAX_CHAINS),
            finalScore,
            m_enemyInfo.estimateMaxScore(currentFrameId + plan.totalFrames()),
            m_enemyInfo.estimateMaxScore(currentFrameId + plan.totalFrames() + 50),
            m_enemyInfo.estimateMaxScore(currentFrameId + plan.totalFrames() + 100));

    LOG(INFO) << plan.decisionText() << " : " << buf;    
    return EvalResult(finalScore, feature.toString() + " : " + buf);
}
