#include "move_evaluator.h"

#include "core/plan/plan.h"

#include "evaluation_parameter.h"
#include "score_collector.h"

template<typename ScoreCollector>
void MoveEvaluator<ScoreCollector>::eval(const RefPlan& plan)
{
    evalFrameFeature(plan.totalFrames(), plan.numChigiri());
}

template<typename ScoreCollector>
void MoveEvaluator<ScoreCollector>::evalFrameFeature(int totalFrames, int numChigiri)
{
    sc_->addScore(TOTAL_FRAMES, totalFrames);
    sc_->addScore(NUM_CHIGIRI, numChigiri);
}

template class MoveEvaluator<FeatureScoreCollector>;
template class MoveEvaluator<SimpleScoreCollector>;
