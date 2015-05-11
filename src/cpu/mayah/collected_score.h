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

struct CollectedSimpleSubScore {
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

typedef CollectedSimpleSubScore CollectedSimpleMoveScore;
typedef CollectedSimpleSubScore CollectedSimpleRensaScore;

struct CollectedSimpleScore {
    double score(EvaluationMode mode) const { return moveScore.score(mode) + rensaScore.score(mode); }

    double score(const CollectedCoef& coef) const
    {
        double s = 0.0;
        for (const auto& mode : ALL_EVALUATION_MODES)
            s += score(mode) * coef.coef(mode);
        return s;
    }

    CollectedSimpleMoveScore moveScore;
    CollectedSimpleRensaScore rensaScore;
};

struct CollectedFeatureMoveScore {
    double score(EvaluationMode mode) const { return simpleScore.score(mode); }
    double score(const CollectedCoef& coef) const { return simpleScore.score(coef); }

    CollectedSimpleMoveScore simpleScore;
    std::map<EvaluationFeatureKey, double> collectedFeatures;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures;
};

struct CollectedFeatureRensaScore {
    double score(EvaluationMode mode) const { return simpleScore.score(mode); }
    double score(const CollectedCoef& coef) const { return simpleScore.score(coef); }

    CollectedSimpleRensaScore simpleScore;
    std::map<EvaluationFeatureKey, double> collectedFeatures;
    std::map<EvaluationSparseFeatureKey, std::vector<int>> collectedSparseFeatures;
    std::string bookname;
    ColumnPuyoList puyosToComplement;
};

struct CollectedFeatureScore {
    double score(EvaluationMode mode) const
    {
        return moveScore.simpleScore.score(mode) + rensaScore.simpleScore.score(mode);
    }
    double score(const CollectedCoef& coef) const
    {
        return moveScore.simpleScore.score(coef) + rensaScore.simpleScore.score(coef);
    }

    double feature(EvaluationFeatureKey key) const
    {
        auto it = moveScore.collectedFeatures.find(key);
        if (it != moveScore.collectedFeatures.end())
            return it->second;

        it = rensaScore.collectedFeatures.find(key);
        if (it != rensaScore.collectedFeatures.end())
            return it->second;

        return 0.0;
    }

    const std::vector<int>& feature(EvaluationSparseFeatureKey key) const
    {
        auto it = moveScore.collectedSparseFeatures.find(key);
        if (it != moveScore.collectedSparseFeatures.end())
            return it->second;

        it = rensaScore.collectedSparseFeatures.find(key);
        if (it != rensaScore.collectedSparseFeatures.end())
            return it->second;

        // This should be thread-safe in C++11.
        static std::vector<int> emptyVector;
        return emptyVector;
    }

    CollectedFeatureMoveScore moveScore;
    CollectedFeatureRensaScore rensaScore;
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
    const std::string& bookName() const { return collectedFeatureScore_.rensaScore.bookname; }

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

    const ColumnPuyoList& puyosToComplement() const { return collectedFeatureScore_.rensaScore.puyosToComplement; }

    std::string toString() const;
    std::string toStringComparingWith(const CollectedFeatureCoefScore&, const EvaluationParameterMap&) const;

private:
    CollectedCoef collectedCoef_;
    CollectedFeatureScore collectedFeatureScore_;
};

#endif
