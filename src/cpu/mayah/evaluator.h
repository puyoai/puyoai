#ifndef EVALUATION_FEATURE_COLLECTOR_H_
#define EVALUATION_FEATURE_COLLECTOR_H_

#include <map>
#include <vector>

#include "feature_parameter.h"

class CoreField;
class Gazer;
class RefPlan;
struct TrackedPossibleRensaInfo;

struct CollectedFeature {
    double score;
    std::map<EvaluationFeatureKey, double> collectedFeatures;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures;
};

class Evaluator {
public:
    static const int NUM_KEY_PUYOS = 1;

    Evaluator(const FeatureParameter& param) : param_(param) {}

    double eval(const RefPlan&, int currentFrameId, const Gazer&);

    // Same as eval(). However,
    CollectedFeature evalWithCollectingFeature(const RefPlan&, int currentFrameId, const Gazer&);

private:
    const FeatureParameter& param_;
};

#endif
