#ifndef CPU_MAYAH_COLLECTED_SCORE_H_
#define CPU_MAYAH_COLLECTED_SCORE_H_

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
    double score(EvaluationMode mode) const
    {
        return moveScore.score(mode) + mainRensaScore.score(mode) + sideRensaScore.score(mode);
    }
    double score(const CollectedCoef& coef) const
    {
        return moveScore.score(coef) + mainRensaScore.score(coef) + sideRensaScore.score(coef);
    }

    CollectedSimpleMoveScore moveScore;
    CollectedSimpleRensaScore mainRensaScore;
    CollectedSimpleRensaScore sideRensaScore;
};

struct CollectedFeatureMoveScore {
    double score(EvaluationMode mode) const { return simpleScore.score(mode); }
    double score(const CollectedCoef& coef) const { return simpleScore.score(coef); }

    double feature(EvaluationMoveFeatureKey key) const
    {
        auto it = collectedFeatures.find(key);
        if (it != collectedFeatures.end())
            return it->second;

        return 0.0;
    }

    const std::vector<int>& feature(EvaluationMoveSparseFeatureKey key) const
    {
        auto it = collectedSparseFeatures.find(key);
        if (it != collectedSparseFeatures.end())
            return it->second;

        // This should be thread-safe in C++11.
        static std::vector<int> emptyVector;
        return emptyVector;
    }

    double scoreFor(EvaluationMoveFeatureKey key,
                    const CollectedCoef& coef,
                    const EvaluationMoveParameterSet& paramSet) const
    {
        double s = 0;
        for (const auto& mode : ALL_EVALUATION_MODES) {
            s += coef.coef(mode) * paramSet.param(mode, key) * feature(key);
        }
        return s;
    }

    double scoreFor(EvaluationMoveSparseFeatureKey key,
                    const CollectedCoef& coef,
                    const EvaluationMoveParameterSet& paramSet) const
    {
        double s = 0;
        for (const auto& mode : ALL_EVALUATION_MODES) {
            for (int v : feature(key)) {
                s += coef.coef(mode) * paramSet.param(mode, key, v);
            }
        }
        return s;
    }

    std::string toString() const;

    CollectedSimpleMoveScore simpleScore;
    std::map<EvaluationMoveFeatureKey, double> collectedFeatures;
    std::map<EvaluationMoveSparseFeatureKey, std::vector<int>> collectedSparseFeatures;
};

struct CollectedFeatureRensaScore {
    double score(EvaluationMode mode) const { return simpleScore.score(mode); }
    double score(const CollectedCoef& coef) const { return simpleScore.score(coef); }

    double feature(EvaluationRensaFeatureKey key) const
    {
        auto it = collectedFeatures.find(key);
        if (it != collectedFeatures.end())
            return it->second;

        return 0.0;
    }

    const std::vector<int>& feature(EvaluationRensaSparseFeatureKey key) const
    {
        auto it = collectedSparseFeatures.find(key);
        if (it != collectedSparseFeatures.end())
            return it->second;

        // This should be thread-safe in C++11.
        static std::vector<int> emptyVector;
        return emptyVector;
    }

    double scoreFor(EvaluationRensaFeatureKey key,
                    const CollectedCoef& coef,
                    const EvaluationRensaParameterSet& paramSet) const
    {
        double s = 0;
        for (const auto& mode : ALL_EVALUATION_MODES) {
            s += coef.coef(mode) * paramSet.param(mode, key) * feature(key);
        }
        return s;
    }

    double scoreFor(EvaluationRensaSparseFeatureKey key,
                    const CollectedCoef& coef,
                    const EvaluationRensaParameterSet& paramSet) const
    {
        double s = 0;
        for (const auto& mode : ALL_EVALUATION_MODES) {
            for (int v : feature(key)) {
                s += coef.coef(mode) * paramSet.param(mode, key, v);
            }
        }
        return s;
    }

    std::string toString() const;

    CollectedSimpleRensaScore simpleScore;
    std::map<EvaluationRensaFeatureKey, double> collectedFeatures;
    std::map<EvaluationRensaSparseFeatureKey, std::vector<int>> collectedSparseFeatures;
    std::string bookname;
    ColumnPuyoList puyosToComplement;
};

struct CollectedFeatureScore {
    double score(EvaluationMode mode) const
    {
        return moveScore.simpleScore.score(mode) +
            mainRensaScore.simpleScore.score(mode) +
            sideRensaScore.simpleScore.score(mode);
    }
    double score(const CollectedCoef& coef) const
    {
        return moveScore.simpleScore.score(coef) +
            mainRensaScore.simpleScore.score(coef) +
            sideRensaScore.simpleScore.score(coef);
    }

    CollectedFeatureMoveScore moveScore;
    CollectedFeatureRensaScore mainRensaScore;
    CollectedFeatureRensaScore sideRensaScore;
};

struct CollectedFeatureCoefScore {
public:
    CollectedFeatureCoefScore(const CollectedCoef& coef,
                              const CollectedFeatureScore& featureScore) :
        collectedCoef_(coef),
        collectedFeatureScore_(featureScore)
    {
    }

    const CollectedCoef& coef() const { return collectedCoef_; }
    double coef(EvaluationMode mode) const { return collectedCoef_.coef(mode); }

    const CollectedFeatureMoveScore& moveScore() const { return collectedFeatureScore_.moveScore; }
    const CollectedFeatureRensaScore& mainRensaScore() const { return collectedFeatureScore_.mainRensaScore; }
    const CollectedFeatureRensaScore& sideRensaScore() const { return collectedFeatureScore_.sideRensaScore; }

    double score() const { return collectedFeatureScore_.score(collectedCoef_); }

    std::string toString() const;
    static std::string scoreComparisionString(const CollectedFeatureCoefScore&,
                                              const CollectedFeatureCoefScore&,
                                              const EvaluationParameterMap&);


private:
    CollectedCoef collectedCoef_;
    CollectedFeatureScore collectedFeatureScore_;
};

#endif // CPU_MAYAH_COLLECTED_SCORE_H_
