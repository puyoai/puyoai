#ifndef MAYAH_AI_FEATURE_COLLECTOR_H_
#define MAYAH_AI_FEATURE_COLLECTOR_H_

#include <map>
#include <string>
#include <vector>

#include "core/column_puyo_list.h"

#include "evaluation_feature.h"
#include "evaluation_parameter.h"

class CollectedFeature {
public:
    CollectedFeature() {}
    CollectedFeature(double score,
                     std::string bookName,
                     std::map<EvaluationFeatureKey, double> collectedFeatures,
                     std::map<EvaluationSparseFeatureKey,
                     std::vector<int>> collectedSparseFeatures,
                     const ColumnPuyoList& rensaKeyPuyos,
                     const ColumnPuyoList& rensaFirePuyos) :
        score_(score),
        bookName_(bookName),
        collectedFeatures_(std::move(collectedFeatures)),
        collectedSparseFeatures_(std::move(collectedSparseFeatures)),
        rensaKeyPuyos_(rensaKeyPuyos),
        rensaFirePuyos_(rensaFirePuyos)
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

    const ColumnPuyoList& rensaKeyPuyos() const { return rensaKeyPuyos_; }
    const ColumnPuyoList& rensaFirePuyos() const { return rensaFirePuyos_; }

    std::string toString() const;
    std::string toStringComparingWith(const CollectedFeature&, const EvaluationParameter&) const;

private:
    static const std::vector<int>& emptyVector()
    {
        static std::vector<int> vs;
        return vs;
    }

    double score_ = 0.0;
    std::string bookName_;
    std::map<EvaluationFeatureKey, double> collectedFeatures_;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures_;
    ColumnPuyoList rensaKeyPuyos_;
    ColumnPuyoList rensaFirePuyos_;
};

// This collector collects all features.
class FeatureCollector {
public:
    FeatureCollector(const EvaluationParameter& param) : param_(param) {}

    void addScore(EvaluationFeatureKey key, double v)
    {
        score_ += param_.score(key, v);
        collectedFeatures_[key] += v;
    }

    void addScore(EvaluationSparseFeatureKey key, int idx, int n)
    {
        score_ += param_.score(key, idx, n);
        for (int i = 0; i < n; ++i)
            collectedSparseFeatures_[key].push_back(idx);
    }

    void merge(const FeatureCollector& fc)
    {
        score_ += fc.score();
        if (bookName_.empty())
            bookName_ = fc.bookName();

        for (const auto& entry : fc.collectedFeatures_) {
            collectedFeatures_[entry.first] = entry.second;
        }
        for (const auto& entry : fc.collectedSparseFeatures_) {
            collectedSparseFeatures_[entry.first].insert(
                collectedSparseFeatures_[entry.first].end(),
                entry.second.begin(),
                entry.second.end());
        }
    }

    void setBookName(const std::string& bookName) { bookName_ = bookName; }
    std::string bookName() const { return bookName_; }

    double score() const { return score_; }
    const EvaluationParameter& evaluationParameter() const { return param_; }

    void setEstimatedRensaScore(int s) { estimatedRensaScore_ = s; }
    int estimatedRensaScore() const { return estimatedRensaScore_; }

    void setRensaKeyPuyos(const ColumnPuyoList& cpl) { rensaKeyPuyos_ = cpl; }
    void setRensaFirePuyos(const ColumnPuyoList& cpl) { rensaFirePuyos_ = cpl; }

    CollectedFeature toCollectedFeature() const {
        return CollectedFeature {
            score(),
            bookName(),
            collectedFeatures_,
            collectedSparseFeatures_,
            rensaKeyPuyos_,
            rensaFirePuyos_
        };
    }

private:
    const EvaluationParameter& param_;
    double score_ = 0.0;
    int estimatedRensaScore_ = 0;
    std::string bookName_;

    std::map<EvaluationFeatureKey, double> collectedFeatures_;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures_;
    ColumnPuyoList rensaKeyPuyos_;
    ColumnPuyoList rensaFirePuyos_;
};

#endif
