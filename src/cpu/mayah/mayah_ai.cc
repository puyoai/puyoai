#include "mayah_ai.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/time.h"
#include "base/wait_group.h"
#include "core/plan/plan.h"
#include "core/frame_request.h"
#include "core/probability/puyo_set_probability.h"

#include "evaluation_parameter.h"
#include "evaluator.h"
#include "gazer.h"

DECLARE_bool(from_wrapper);

using namespace std;

MayahAI::MayahAI(int argc, char* argv[], std::unique_ptr<Executor> executor) :
    MayahBaseAI(argc, argv, "mayah", std::move(executor))
{
    if (!FLAGS_from_wrapper) {
        LOG(ERROR) << "mayah was not run with run.sh?" << endl
                   << "Use run.sh instead of using mayah_cpu directly.";
    }

    setBehaviorRethinkAfterOpponentRensa(true);
}

MayahAI::~MayahAI()
{
}

DropDecision MayahAI::think(int frame_id, const CoreField& f, const KumipuyoSeq& kumipuyo_seq,
                            const PlayerState& me, const PlayerState& enemy, bool fast) const
{
    return pattern_thinker_->think(frame_id, f, kumipuyo_seq, me, enemy, gazer_.gazeResult(), fast,
                                   usesDecisionBook_, usesRensaHandTree_);
}

ThoughtResult MayahAI::thinkPlan(int frameId, const CoreField& cf, const KumipuyoSeq& seq,
                                 const PlayerState& me, const PlayerState& enemy,
                                 int depth, int maxIteration, bool fast,
                                 std::vector<Decision>* specifiedDecisions) const
{
    return pattern_thinker_->thinkPlan(frameId, cf, seq, me, enemy, depth, maxIteration, gazer_.gazeResult(), fast,
                                       usesDecisionBook_, usesRensaHandTree_, specifiedDecisions);
}

CollectedFeatureCoefScore MayahAI::evalWithCollectingFeature(
    const RefPlan& plan, const KumipuyoSeq& restSeq, int currentFrameId, int maxIteration,
    const PlayerState& me, const PlayerState& enemy,
    const MidEvalResult& midEvalResult, bool fast, const GazeResult& gazeResult) const
{
    return pattern_thinker_->evalWithCollectingFeature(plan, restSeq, currentFrameId, maxIteration,
                                                       me, enemy, midEvalResult, fast,
                                                       usesRensaHandTree_, gazeResult);
}

void DebuggableMayahAI::setEvaluationParameterMap(const EvaluationParameterMap& map)
{
    evaluationParameterMap_.loadValue(map.toTomlValue());
}
