#include "ai_routine.h"

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/client/connector/drop_decision.h"
#include "core/frame_data.h"
#include "cpu/mayah/evaluation_feature.h"
#include "cpu/mayah/evaluation_feature_collector.h"
#include "enemy_info.h"

using namespace std;

EvalResult::EvalResult(double evaluation, const string& message) :
    evaluationScore(evaluation),
    message(message)
{
    for (string::size_type i = 0; i < this->message.size(); ++i) {
        if (this->message[i] == ' ')
            this->message[i] = '_';
    }
}

AIRoutine::AIRoutine() :
    AI("mayah"),
    evaluationParams_("feature.txt")
{
    LOG(INFO) << evaluationParams_.toString();
}

void AIRoutine::gameWillBegin(const FrameData& frameData)
{
    enemyInfo_.initializeWith(frameData.id);
}

void AIRoutine::gameHasEnd(const FrameData&)
{
}

DropDecision AIRoutine::think(int frameId, const PlainField& plainField, const Kumipuyo& next1, const Kumipuyo& next2)
{
    CoreField field(plainField);
    KumipuyoSeq kumipuyoSeq { next1, next2 };

    LOG(INFO) << "\n" << field.debugOutput();
    LOG(INFO) << next1.toString() << " " << next2.toString();

    std::vector<Plan> plans = Plan::findAvailablePlans(field, kumipuyoSeq);

    double currentBestScore = -100.0;
    DropDecision dropDecision;
    for (auto it = plans.cbegin(); it != plans.cend(); ++it) {
        EvalResult result = eval(frameId, *it);
        if (currentBestScore < result.evaluationScore) {
            currentBestScore = result.evaluationScore;
            dropDecision = DropDecision(it->firstDecision(), result.message);
        }
    }

    LOG(INFO) << "Decided : " << dropDecision.decision().toString();
    return dropDecision;
}

void AIRoutine::enemyGrounded(const FrameData& frameData)
{
    enemyInfo_.setId(frameData.id);

    // --- Check if Rensa starts.
    CoreField field(frameData.enemyPlayerFrameData().field);
    field.forceDrop();

    BasicRensaResult rensaInfo = field.simulate();

    if (rensaInfo.chains > 0)
        enemyInfo_.setOngoingRensa(OngoingRensaInfo(rensaInfo, frameData.id + rensaInfo.frames));
    else
        enemyInfo_.setRensaIsOngoing(false);
}

void AIRoutine::enemyNext2Appeared(const FrameData& frameData)
{
    int currentFrameId = frameData.id;

    enemyInfo_.setId(currentFrameId);

    enemyInfo_.updateFeasibleRensas(frameData.enemyPlayerFrameData().field, frameData.enemyPlayerFrameData().kumipuyoSeq);
    enemyInfo_.updatePossibleRensas(frameData.enemyPlayerFrameData().field, frameData.enemyPlayerFrameData().kumipuyoSeq);

    LOG(INFO) << "Possible rensa infos : ";
    for (auto it = enemyInfo_.possibleRensaInfos().begin(); it != enemyInfo_.possibleRensaInfos().end(); ++it)
        LOG(INFO) << it->toString();
    LOG(INFO) << "Feasible rensa infos : ";
    for (auto it = enemyInfo_.feasibleRensaInfos().begin(); it != enemyInfo_.feasibleRensaInfos().end(); ++it)
        LOG(INFO) << it->toString();
}

EvalResult AIRoutine::eval(int currentFrameId, const Plan& plan) const
{
    EvaluationFeature feature;
    EvaluationFeatureCollector::collectFeatures(feature, plan, NUM_KEY_PUYOS, currentFrameId, enemyInfo_);

    const RensaEvaluationFeature& bestRensaFeature = feature.findBestRensaFeature(evaluationParams_);
    double finalScore = feature.calculateScoreWith(evaluationParams_, bestRensaFeature);

    char buf[256];
    sprintf(buf, "max rensa = %d : eval score = %f : enemy = %d : %d : %d",
            bestRensaFeature.get(MAX_CHAINS),
            finalScore,
            enemyInfo_.estimateMaxScore(currentFrameId + plan.totalFrames()),
            enemyInfo_.estimateMaxScore(currentFrameId + plan.totalFrames() + 50),
            enemyInfo_.estimateMaxScore(currentFrameId + plan.totalFrames() + 100));

    // LOG(INFO) << plan.decisionText() << " : " << buf;
    return EvalResult(finalScore, feature.toString() + " : " + buf);
}

