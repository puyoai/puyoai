#ifndef MAYAH_AI_COLLECTED_SCORE_H_
#define MAYAH_AI_COLLECTED_SCORE_H_

#include <array>

#include "core/column_puyo_list.h"

#include "evaluation_feature.h"
#include "evaluation_parameter.h"

struct CollectedCoef {
    double coef(EvaluationMode mode) const { return coefMap[ordinal(mode)]; }
    void setCoef(EvaluationMode mode, double x) { coefMap[ordinal(mode)] = x; }

    std::array<double, NUM_EVALUATION_MODES> coefMap {{}};
};

struct CollectedSimpleScore {
    double score(EvaluationMode mode) const { return scoreMap[ordinal(mode)]; }
    double score(const CollectedCoef& coef) const
    {
        double s = 0.0;
        for (const auto& mode : ALL_EVALUATION_MODES)
            s += score(mode) * coef.coef(mode);
        return s;
    }

    std::array<double, NUM_EVALUATION_MODES> scoreMap {{}};
};

struct CollectedFeatureScore {
    double score(EvaluationMode mode) const { return collectedSimpleScore.score(mode); }
    double score(const CollectedCoef& coef) const { return collectedSimpleScore.score(coef); }

    double feature(EvaluationFeatureKey key) const
    {
        auto it = collectedFeatures.find(key);
        if (it != collectedFeatures.end())
            return it->second;
        return 0.0;
    }

    const std::vector<int>& feature(EvaluationSparseFeatureKey key) const
    {
        auto it = collectedSparseFeatures.find(key);
        if (it != collectedSparseFeatures.end())
            return it->second;

        // This should be thread-safe in C++11.
        static std::vector<int> emptyVector;
        return emptyVector;
    }

    CollectedSimpleScore collectedSimpleScore;
    std::string bookName;
    std::map<EvaluationFeatureKey, double> collectedFeatures;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures;
    ColumnPuyoList puyosToComplement;
};

struct CollectedFeatureCoefScore {
public:
    CollectedFeatureCoefScore(const CollectedCoef& coef,
                              const CollectedFeatureScore& featureScore) :
        collectedCoef_(coef),
        collectedFeatureScore_(featureScore)
    {
    }

    double coef(EvaluationMode mode) const { return collectedCoef_.coef(mode); }
    double feature(EvaluationFeatureKey key) const { return collectedFeatureScore_.feature(key); }
    const std::vector<int>& feature(EvaluationSparseFeatureKey key) const { return collectedFeatureScore_.feature(key); }
    const std::string& bookName() const { return collectedFeatureScore_.bookName; }

    double score() const { return collectedFeatureScore_.score(collectedCoef_); }

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

    const ColumnPuyoList& puyosToComplement() const { return collectedFeatureScore_.puyosToComplement; }

    std::string toString() const;
    std::string toStringComparingWith(const CollectedFeatureCoefScore&, const EvaluationParameterMap&) const;

private:
    CollectedCoef collectedCoef_;
    CollectedFeatureScore collectedFeatureScore_;
};

#endif
