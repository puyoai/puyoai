#ifndef EVALUATION_FEATURE_COLLECTOR_H_
#define EVALUATION_FEATURE_COLLECTOR_H_

#include <map>
#include <vector>

#include "book_field.h"
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

    std::string toString() const;
    std::string toStringComparingWith(const CollectedFeature&) const;

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

class NormalScoreCollector {
public:
    explicit NormalScoreCollector(const FeatureParameter& param) : param_(param) {}

    void addScore(EvaluationFeatureKey key, double v) { score_ += param_.score(key, v); }
    void addScore(EvaluationSparseFeatureKey key, int idx, int n) { score_ += param_.score(key, idx, n); }
    void merge(const NormalScoreCollector& sc) { score_ += sc.score(); }

    double score() const { return score_; }
    const FeatureParameter& featureParameter() const { return param_; }

private:
    const FeatureParameter& param_;
    double score_ = 0.0;
};

class FeatureScoreCollector {
public:
    FeatureScoreCollector(const FeatureParameter& param) : collector_(param) {}

    void addScore(EvaluationFeatureKey key, double v)
    {
        collector_.addScore(key, v);
        collectedFeatures_[key] = v;
    }

    void addScore(EvaluationSparseFeatureKey key, int idx, int n)
    {
        collector_.addScore(key, idx, n);
        for (int i = 0; i < n; ++i)
            collectedSparseFeatures_[key].push_back(idx);
    }

    void merge(const FeatureScoreCollector& sc)
    {
        collector_.merge(sc.collector_);

        for (const auto& entry : sc.collectedFeatures_) {
            collectedFeatures_[entry.first] = entry.second;
        }
        for (const auto& entry : sc.collectedSparseFeatures_) {
            collectedSparseFeatures_[entry.first].insert(
                collectedSparseFeatures_[entry.first].end(),
                entry.second.begin(),
                entry.second.end());
        }
    }

    double score() const { return collector_.score(); }
    const FeatureParameter& featureParameter() const { return collector_.featureParameter(); }

    CollectedFeature toCollectedFeature() const {
        return CollectedFeature {
            score(),
            collectedFeatures_,
            collectedSparseFeatures_
        };
    }

private:
    NormalScoreCollector collector_;
    std::map<EvaluationFeatureKey, double> collectedFeatures_;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures_;
};

class Evaluator {
public:
    Evaluator(const FeatureParameter& param, const std::vector<BookField>& books) : param_(param), books_(books) {}

    double eval(const RefPlan&, const CoreField& currentField, int currentFrameId, int numKeyPuyos, const Gazer&);
    // Same as eval(), but returns CollectedFeature.
    CollectedFeature evalWithCollectingFeature(const RefPlan&, const CoreField& currentField, int currentFrameId, int numKeyPuyos, const Gazer&);

private:
    const FeatureParameter& param_;
    const std::vector<BookField>& books_;
};

#include "evaluator_inl.h"

#endif
