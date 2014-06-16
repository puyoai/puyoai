#include "ai_routine.h"

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/client/connector/drop_decision.h"
#include "core/frame_data.h"

#include "evaluation_feature.h"
#include "evaluator.h"
#include "gazer.h"

using namespace std;

AIRoutine::AIRoutine() :
    AI("mayah")
{
    feature_.reset(new EvaluationFeature("feature.txt"));
    LOG(INFO) << feature_->toString();
    evaluator_.reset(new Evaluator);
}

AIRoutine::~AIRoutine()
{
}

void AIRoutine::gameWillBegin(const FrameData& frameData)
{
    gazer_.initializeWith(frameData.id);
}

void AIRoutine::gameHasEnded(const FrameData&)
{
}

DropDecision AIRoutine::think(int frameId, const PlainField& plainField, const Kumipuyo& next1, const Kumipuyo& next2)
{
    CoreField field(plainField);
    KumipuyoSeq kumipuyoSeq { next1, next2 };

    LOG(INFO) << "\n" << field.debugOutput();
    LOG(INFO) << next1.toString() << " " << next2.toString();

    double bestScore = -100.0;
    DropDecision dropDecision;
    Plan::iterateAvailablePlans(field, kumipuyoSeq, 2,
                                [this, frameId, &bestScore, &dropDecision](const RefPlan& plan) {
            EvalResult result = evaluator_->eval(*feature_, plan, frameId, gazer_);
            if (bestScore < result.evaluationScore) {
                bestScore = result.evaluationScore;
                dropDecision = DropDecision(plan.decisions().front(), result.message);
            }
    });

    LOG(INFO) << "Decided : " << dropDecision.decision().toString();
    return dropDecision;
}

void AIRoutine::enemyGrounded(const FrameData& frameData)
{
    gazer_.setId(frameData.id);

    // --- Check if Rensa starts.
    CoreField field(frameData.enemyPlayerFrameData().field);
    field.forceDrop();

    RensaResult rensaResult = field.simulate();

    if (rensaResult.chains > 0)
        gazer_.setOngoingRensa(OngoingRensaInfo(rensaResult, frameData.id + rensaResult.frames));
    else
        gazer_.setRensaIsOngoing(false);
}

void AIRoutine::enemyNext2Appeared(const FrameData& frameData)
{
    int currentFrameId = frameData.id;

    gazer_.setId(currentFrameId);
    gazer_.updateFeasibleRensas(frameData.enemyPlayerFrameData().field, frameData.enemyPlayerFrameData().kumipuyoSeq);
    gazer_.updatePossibleRensas(frameData.enemyPlayerFrameData().field, frameData.enemyPlayerFrameData().kumipuyoSeq);

    LOG(INFO) << "Possible rensa infos : ";
    for (auto it = gazer_.possibleRensaInfos().begin(); it != gazer_.possibleRensaInfos().end(); ++it)
        LOG(INFO) << it->toString();
    LOG(INFO) << "Feasible rensa infos : ";
    for (auto it = gazer_.feasibleRensaInfos().begin(); it != gazer_.feasibleRensaInfos().end(); ++it)
        LOG(INFO) << it->toString();
}
