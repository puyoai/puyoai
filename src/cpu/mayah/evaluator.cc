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

string CollectedFeature::toString() const
{
    stringstream ss;
    ss << "score = " << score_ << endl;
    for (const auto& entry : collectedFeatures_) {
        ss << ::toString(entry.first) << "=" << to_string(entry.second) << endl;
    }
    for (const auto& entry : collectedSparseFeatures_) {
        ss << ::toString(entry.first) << "=";
        for (int v : entry.second)
            ss << v << ' ';
        ss << endl;
    }

    return ss.str();
}

string CollectedFeature::toStringComparingWith(const CollectedFeature& cf) const
{
    set<EvaluationFeatureKey> keys;
    for (const auto& entry : collectedFeatures_) {
        keys.insert(entry.first);
    }
    for (const auto& entry : cf.collectedFeatures_) {
        keys.insert(entry.first);
    }

    set<EvaluationSparseFeatureKey> sparseKeys;
    for (const auto& entry : collectedSparseFeatures_) {
        sparseKeys.insert(entry.first);
    }
    for (const auto& entry : cf.collectedSparseFeatures_) {
        sparseKeys.insert(entry.first);
    }

    stringstream ss;
    ss << "score = " << score_ << " : " << cf.score_ << endl;
    for (EvaluationFeatureKey key : keys) {
        ss << ::toString(key) << " = "
           << to_string(feature(key)) << " : "
           << to_string(cf.feature(key)) << endl;
    }
    for (EvaluationSparseFeatureKey key : sparseKeys) {
        ss << ::toString(key) << " =";
        for (int v : feature(key)) {
            ss << " " << v;
        }
        ss << " :";
        for (int v : cf.feature(key)) {
            ss << " " << v;
        }
        ss << endl;
    }

    return ss.str();
}

double Evaluator::eval(const RefPlan& plan, const CoreField& currentField,
                       int currentFrameId, int maxIteration, const Gazer& gazer)
{
    NormalScoreCollector sc(param_);
    collectScore(&sc, books_, plan, currentField, currentFrameId, maxIteration, gazer);
    return sc.score();
}

CollectedFeature Evaluator::evalWithCollectingFeature(const RefPlan& plan, const CoreField& currentField,
                                                      int currentFrameId, int maxIteration, const Gazer& gazer)
{
    FeatureScoreCollector sc(param_);
    collectScore(&sc, books_, plan, currentField, currentFrameId, maxIteration, gazer);
    return sc.toCollectedFeature();
}
