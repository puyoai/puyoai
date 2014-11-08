#ifndef MAYAH_AI_SCORE_COLLECTOR_H_
#define MAYAH_AI_SCORE_COLLECTOR_H_

#include <map>
#include <string>
#include <vector>

#include "evaluation_feature.h"
#include "evaluation_parameter.h"

class CollectedFeature {
public:
    CollectedFeature() {}
    CollectedFeature(double score,
                     std::string bookName,
                     std::map<EvaluationFeatureKey, double> collectedFeatures,
                     std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures) :
        score_(score),
        bookName_(bookName),
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

    const std::string& bookName() const { return bookName_; }

    std::string toString() const;
    std::string toStringComparingWith(const CollectedFeature&) const;

    static const std::vector<int>& emptyVector()
    {
        static std::vector<int> vs;
        return vs;
    }

private:
    double score_ = 0.0;
    std::string bookName_;
    std::map<EvaluationFeatureKey, double> collectedFeatures_;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures_;
};

// This collector collects score and bookname.
class NormalScoreCollector {
public:
    explicit NormalScoreCollector(const EvaluationParameter& param) : param_(param) {}

    void addScore(EvaluationFeatureKey key, double v) { score_ += param_.score(key, v); }
    void addScore(EvaluationSparseFeatureKey key, int idx, int n) { score_ += param_.score(key, idx, n); }
    void merge(const NormalScoreCollector& sc) { score_ += sc.score(); }

    void setBookName(const std::string& bookName) { bookName_ = bookName; }
    std::string bookName() const { return bookName_; }

    double score() const { return score_; }
    const EvaluationParameter& evaluationParameter() const { return param_; }

    void setEstimatedRensaScore(int s) { estimatedRensaScore_ = s; }
    int estimatedRensaScore() const { return estimatedRensaScore_; }

private:
    const EvaluationParameter& param_;
    double score_ = 0.0;
    int estimatedRensaScore_ = 0;
    std::string bookName_;
};

// This collector collects all features.
class FeatureScoreCollector {
public:
    FeatureScoreCollector(const EvaluationParameter& param) : collector_(param) {}

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

    void setBookName(const std::string& bookName) { collector_.setBookName(bookName); }
    std::string bookName() const { return collector_.bookName(); }

    double score() const { return collector_.score(); }
    const EvaluationParameter& evaluationParameter() const { return collector_.evaluationParameter(); }

    void setEstimatedRensaScore(int s) { collector_.setEstimatedRensaScore(s); }
    int estimatedRensaScore() const { return collector_.estimatedRensaScore(); }

    CollectedFeature toCollectedFeature() const {
        return CollectedFeature {
            score(),
            bookName(),
            collectedFeatures_,
            collectedSparseFeatures_
        };
    }

private:
    NormalScoreCollector collector_;
    std::map<EvaluationFeatureKey, double> collectedFeatures_;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures_;
};

#endif
