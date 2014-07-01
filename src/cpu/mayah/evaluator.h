#ifndef EVALUATION_FEATURE_COLLECTOR_H_
#define EVALUATION_FEATURE_COLLECTOR_H_

#include <map>
#include <vector>

#include "feature_parameter.h"

class CoreField;
class Gazer;
class RefPlan;
struct TrackedPossibleRensaInfo;

class CollectedFeature {
public:
    CollectedFeature() {}
    CollectedFeature(double score,
                     std::map<EvaluationFeatureKey, double> collectedFeatures,
                     std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures) :
        score_(score),
        collectedFeatures_(std::move(collectedFeatures)),
        collectedSparseFeatures_(std::move(collectedSparseFeatures))
    {
    }


    double score() const { return score_; }
    double feature(EvaluationFeatureKey key) const
    {
        auto it = collectedFeatures_.find(key);
        if (it != collectedFeatures_.end())
            return it->second;
        return 0.0;
    }

    const std::vector<int>& feature(EvaluationSparseFeatureKey key) const
    {
        auto it = collectedSparseFeatures_.find(key);
        if (it != collectedSparseFeatures_.end())
            return it->second;

        return emptyVector();
    }

    static const std::vector<int>& emptyVector()
    {
        static std::vector<int> vs;
        return vs;
    }

private:
    double score_ = 0.0;
    std::map<EvaluationFeatureKey, double> collectedFeatures_;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures_;
};

class Evaluator {
public:
    static const int NUM_KEY_PUYOS = 1;

    Evaluator(const FeatureParameter& param) : param_(param) {}

    double eval(const RefPlan&, const CoreField& currentField, int currentFrameId, bool fast, const Gazer&);

    // Same as eval(), but returns CollectedFeature.
    CollectedFeature evalWithCollectingFeature(const RefPlan&, const CoreField& currentField, int currentFrameId, bool fast, const Gazer&);

private:
    const FeatureParameter& param_;
};

#endif
