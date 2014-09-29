#include "evaluator.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>

#include <glog/logging.h>

#include "base/time.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/algorithm/rensa_info.h"
#include "core/algorithm/rensa_detector.h"
#include "core/constant.h"
#include "core/decision.h"
#include "core/position.h"
#include "core/field/core_field.h"
#include "core/field/field_bit_field.h"
#include "core/field/rensa_result.h"
#include "core/score.h"

#include "feature_parameter.h"
#include "gazer.h"

using namespace std;

EvalResult Evaluator::eval(const RefPlan& plan, const CoreField& currentField,
                           int currentFrameId, int maxIteration, const Gazer& gazer)
{
    NormalScoreCollector sc(param_);
    collectScore(&sc, books_, plan, currentField, currentFrameId, maxIteration, gazer);

    return EvalResult(sc.score(), sc.estimatedRensaScore());
}

CollectedFeature Evaluator::evalWithCollectingFeature(const RefPlan& plan, const CoreField& currentField,
                                                      int currentFrameId, int maxIteration, const Gazer& gazer)
{
    FeatureScoreCollector sc(param_);
    collectScore(&sc, books_, plan, currentField, currentFrameId, maxIteration, gazer);
    return sc.toCollectedFeature();
}
