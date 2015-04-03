#ifndef MAYAH_AI_SCORE_COLLECTOR_H_
#define MAYAH_AI_SCORE_COLLECTOR_H_

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
                     const std::array<double, ARRAY_SIZE(ALL_EVALUATION_MODES)>& scoreCoef,
                     std::string bookName,
                     std::map<EvaluationFeatureKey, double> collectedFeatures,
                     std::map<EvaluationSparseFeatureKey,
                     std::vector<int>> collectedSparseFeatures,
                     const ColumnPuyoList& rensaKeyPuyos,
                     const ColumnPuyoList& rensaFirePuyos) :
        score_(score),
        scoreCoef_(scoreCoef),
        bookName_(bookName),
        collectedFeatures_(std::move(collectedFeatures)),
        collectedSparseFeatures_(std::move(collectedSparseFeatures)),
        rensaKeyPuyos_(rensaKeyPuyos),
        rensaFirePuyos_(rensaFirePuyos)
    {
    }

    double score() const { return score_; }

    double coef(EvaluationMode mode) const { return scoreCoef_[ordinal(mode)]; }

    double scoreFor(EvaluationFeatureKey key, const EvaluationParameterMap& paramMap) const
    {
        double s = 0;
        for (const auto& mode : ALL_EVALUATION_MODES) {
            const EvaluationParameter& param = paramMap.parameter(mode);
            s += param.score(key, feature(key)) * coef(mode);
        }
        return s;
    }
    double scoreFor(EvaluationSparseFeatureKey key, const EvaluationParameterMap& paramMap) const
    {
        double s = 0;
        for (const auto& mode : ALL_EVALUATION_MODES) {
            const EvaluationParameter& param = paramMap.parameter(mode);
            for (int v : feature(key)) {
                s += param.score(key, v, 1) * coef(mode);
            }
        }
        return s;
    }

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
    std::string toStringComparingWith(const CollectedFeature&, const EvaluationParameterMap&) const;

private:
    static const std::vector<int>& emptyVector()
    {
        static std::vector<int> vs;
        return vs;
    }

    double score_ = 0.0;
    std::array<double, ARRAY_SIZE(ALL_EVALUATION_MODES)> scoreCoef_ {};
    std::string bookName_;
    std::map<EvaluationFeatureKey, double> collectedFeatures_;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures_;
    ColumnPuyoList rensaKeyPuyos_;
    ColumnPuyoList rensaFirePuyos_;
};

// This collector collects score and bookname.
class NormalScoreCollector {
public:
    explicit NormalScoreCollector(const EvaluationParameterMap& paramMap) :
        paramMap_(paramMap),
        scoreMap_ {}
    {}

    void addScore(EvaluationFeatureKey key, double v)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            scoreMap_[ordinal(mode)] += paramMap_.parameter(mode).score(key, v);
        }
    }
    void addScore(EvaluationSparseFeatureKey key, int idx, int n)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            scoreMap_[ordinal(mode)] += paramMap_.parameter(mode).score(key, idx, n);
        }
    }
    void merge(const NormalScoreCollector& sc)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            scoreMap_[ordinal(mode)] += sc.scoreMap_[ordinal(mode)];
        }
        if (bookName_.empty())
            bookName_ = sc.bookName();
    }

    void setBookName(const std::string& bookName) { bookName_ = bookName; }
    std::string bookName() const { return bookName_; }

    double score() const { return scoreMap_[ordinal(mode_)]; }
    EvaluationMode mode() const { return mode_; }

    std::array<double, ARRAY_SIZE(ALL_EVALUATION_MODES)> scoreCoef() const
    {
        std::array<double, ARRAY_SIZE(ALL_EVALUATION_MODES)> coef {};
        coef[ordinal(mode_)] = 1.0;
        return coef;
    }

    const EvaluationParameterMap& evaluationParameterMap() const { return paramMap_; }

    void setMode(EvaluationMode m) { mode_ = m; }

    void setEstimatedRensaScore(int s) { estimatedRensaScore_ = s; }
    int estimatedRensaScore() const { return estimatedRensaScore_; }

    void setRensaKeyPuyos(const ColumnPuyoList&) {}
    void setRensaFirePuyos(const ColumnPuyoList&) {}

private:
    const EvaluationParameterMap& paramMap_;
    std::array<double, ARRAY_SIZE(ALL_EVALUATION_MODES)> scoreMap_; // score of each mode.
    EvaluationMode mode_ = EvaluationMode::DEFAULT;
    int estimatedRensaScore_ = 0;
    std::string bookName_;
};

// This collector collects all features.
class FeatureScoreCollector {
public:
    FeatureScoreCollector(const EvaluationParameterMap& paramMap) : collector_(paramMap) {}

    void addScore(EvaluationFeatureKey key, double v)
    {
        collector_.addScore(key, v);
        collectedFeatures_[key] += v;
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
    const EvaluationParameterMap& evaluationParameterMap() const { return collector_.evaluationParameterMap(); }
    std::array<double, ARRAY_SIZE(ALL_EVALUATION_MODES)> scoreCoef() const { return collector_.scoreCoef(); }

    EvaluationMode mode() const { return collector_.mode(); }
    void setMode(EvaluationMode mode) { collector_.setMode(mode); }

    void setEstimatedRensaScore(int s) { collector_.setEstimatedRensaScore(s); }
    int estimatedRensaScore() const { return collector_.estimatedRensaScore(); }

    void setRensaKeyPuyos(const ColumnPuyoList& cpl) { rensaKeyPuyos_ = cpl; }
    void setRensaFirePuyos(const ColumnPuyoList& cpl) { rensaFirePuyos_ = cpl; }

    CollectedFeature toCollectedFeature() const {
        return CollectedFeature {
            score(),
            scoreCoef(),
            bookName(),
            collectedFeatures_,
            collectedSparseFeatures_,
            rensaKeyPuyos_,
            rensaFirePuyos_
        };
    }

private:
    NormalScoreCollector collector_;
    std::map<EvaluationFeatureKey, double> collectedFeatures_;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures_;
    ColumnPuyoList rensaKeyPuyos_;
    ColumnPuyoList rensaFirePuyos_;
};

#endif
