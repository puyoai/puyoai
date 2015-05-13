#ifndef CPU_MAYAH_SCORE_COLLECTOR_H_
#define CPU_MAYAH_SCORE_COLLECTOR_H_

#include <array>
#include <map>
#include <string>
#include <vector>

#include "core/column_puyo_list.h"

#include "collected_score.h"
#include "evaluation_feature.h"
#include "evaluation_parameter.h"

class SimpleRensaScoreCollector {
public:
    typedef CollectedSimpleRensaScore CollectedScore;
    explicit SimpleRensaScoreCollector(const EvaluationRensaParameterSet& paramSet) :
        paramSet_(paramSet)
    {
    }

    void addScore(EvaluationRensaFeatureKey key, double v)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            rensaScore_.scoreMap[ordinal(mode)] += paramSet_.param(mode, key) * v;
        }
    }

    void addScore(EvaluationRensaSparseFeatureKey key, int idx, int n = 1)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            rensaScore_.scoreMap[ordinal(mode)] += paramSet_.param(mode, key, idx) * n;
        }
    }

    void setBookname(const std::string&) {}
    void setPuyosToComplement(const ColumnPuyoList&) {}

    const CollectedScore& collectedScore() const { return rensaScore_; }

private:
    const EvaluationRensaParameterSet& paramSet_;
    CollectedSimpleRensaScore rensaScore_;
};

// This collector collects only score.
class SimpleScoreCollector {
public:
    typedef CollectedSimpleScore CollectedScore;
    typedef SimpleRensaScoreCollector RensaScoreCollector;

    explicit SimpleScoreCollector(const EvaluationParameterMap& paramMap) :
        paramMap_(paramMap)
    {
    }

    void addScore(EvaluationMoveFeatureKey key, double v)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            collectedSimpleScore_.moveScore.scoreMap[ordinal(mode)] += moveParamSet().param(mode, key) * v;
        }
    }

    void addScore(EvaluationMoveSparseFeatureKey key, int idx, int n = 1)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            collectedSimpleScore_.moveScore.scoreMap[ordinal(mode)] += moveParamSet().param(mode, key, idx) * n;
        }
    }

    void merge(const CollectedSimpleRensaScore& rensaScore)
    {
        collectedSimpleScore_.rensaScore = rensaScore;
    }

    void setCoef(const CollectedCoef& coef) { collectedCoef_ = coef; }
    const CollectedCoef& collectedCoef() const { return collectedCoef_; }

    const CollectedSimpleScore& collectedScore() const { return collectedSimpleScore_; }

    const EvaluationMoveParameterSet& moveParamSet() const { return paramMap_.moveParamSet(); }
    const EvaluationRensaParameterSet& rensaParamSet() const { return paramMap_.rensaParamSet(); }

    void setEstimatedRensaScore(int s) { estimatedRensaScore_ = s; }
    int estimatedRensaScore() const { return estimatedRensaScore_; }

private:
    const EvaluationParameterMap& paramMap_;
    CollectedCoef collectedCoef_;
    CollectedSimpleScore collectedSimpleScore_;
    int estimatedRensaScore_ = 0;
};

// ----------------------------------------------------------------------

class FeatureRensaScoreCollector {
public:
    typedef CollectedFeatureRensaScore CollectedScore;
    explicit FeatureRensaScoreCollector(const EvaluationRensaParameterSet& paramSet) :
        paramSet_(paramSet)
    {
    }

    void addScore(EvaluationRensaFeatureKey key, double v)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            rensaScore_.simpleScore.scoreMap[ordinal(mode)] += paramSet_.param(mode, key) * v;
        }
        rensaScore_.collectedFeatures[key] += v;
    }

    void addScore(EvaluationRensaSparseFeatureKey key, int idx, int n = 1)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            rensaScore_.simpleScore.scoreMap[ordinal(mode)] += paramSet_.param(mode, key, idx) * n;
        }
        for (int i = 0; i < n; ++i)
            rensaScore_.collectedSparseFeatures[key].push_back(idx);
    }

    void setBookname(const std::string& bookname) { rensaScore_.bookname = bookname; }
    void setPuyosToComplement(const ColumnPuyoList& cpl) { rensaScore_.puyosToComplement = cpl; }

    const CollectedScore& collectedScore() const { return rensaScore_; }

private:
    const EvaluationRensaParameterSet& paramSet_;
    CollectedFeatureRensaScore rensaScore_;
};

// This collector collects all features.
class FeatureScoreCollector {
public:
    typedef CollectedFeatureScore CollectedScore;
    typedef FeatureRensaScoreCollector RensaScoreCollector;

    explicit FeatureScoreCollector(const EvaluationParameterMap& paramMap) : paramMap_(paramMap) {}

    void addScore(EvaluationMoveFeatureKey key, double v)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            collectedFeatureScore_.moveScore.simpleScore.scoreMap[ordinal(mode)] += moveParamSet().param(mode, key) * v;
        }
        collectedFeatureScore_.moveScore.collectedFeatures[key] += v;
    }

    void addScore(EvaluationMoveSparseFeatureKey key, int idx, int n = 1)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            collectedFeatureScore_.moveScore.simpleScore.scoreMap[ordinal(mode)] += moveParamSet().param(mode, key, idx) * n;
        }
        for (int i = 0; i < n; ++i)
            collectedFeatureScore_.moveScore.collectedSparseFeatures[key].push_back(idx);
    }

    void merge(const CollectedFeatureRensaScore& rensaScore)
    {
        collectedFeatureScore_.rensaScore = rensaScore;
    }

    void setCoef(const CollectedCoef& coef) { collectedCoef_ = coef; }
    const CollectedCoef& collectedCoef() const { return collectedCoef_; }
    const CollectedFeatureScore& collectedScore() const { return collectedFeatureScore_; }

    const EvaluationMoveParameterSet& moveParamSet() const { return paramMap_.moveParamSet(); }
    const EvaluationRensaParameterSet& rensaParamSet() const { return paramMap_.rensaParamSet(); }

    void setEstimatedRensaScore(int s) { estimatedRensaScore_ = s; }
    int estimatedRensaScore() const { return estimatedRensaScore_; }

private:
    const EvaluationParameterMap& paramMap_;
    CollectedCoef collectedCoef_;
    CollectedFeatureScore collectedFeatureScore_;
    int estimatedRensaScore_ = 0;
};

#endif // CPU_MAYAH_SCORE_COLLECTOR_H_
