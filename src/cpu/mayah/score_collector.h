#ifndef MAYAH_AI_SCORE_COLLECTOR_H_
#define MAYAH_AI_SCORE_COLLECTOR_H_

#include <array>
#include <map>
#include <string>
#include <vector>

#include "core/column_puyo_list.h"

#include "collected_score.h"
#include "evaluation_feature.h"
#include "evaluation_parameter.h"

// This collector collects score and bookname.
class SimpleScoreCollector {
public:
    typedef CollectedSimpleScore CollectedScore;

    explicit SimpleScoreCollector(const EvaluationParameterMap& paramMap) :
        paramMap_(paramMap)
    {
    }

    void setMode(EvaluationMode mode)
    {
        for (const auto& m : ALL_EVALUATION_MODES)
            collectedCoef_.coefMap[ordinal(m)] = 0.0;
        collectedCoef_.coefMap[ordinal(mode)] = 1.0;
    }

    void addScore(EvaluationFeatureKey key, double v)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            collectedSimpleScore_.scoreMap[ordinal(mode)] += paramMap_.parameter(mode).score(key, v);
        }
    }

    void addScore(EvaluationSparseFeatureKey key, int idx, int n)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            collectedSimpleScore_.scoreMap[ordinal(mode)] += paramMap_.parameter(mode).score(key, idx, n);
        }
    }
    void merge(const CollectedSimpleScore& collectedSimpleScore)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            collectedSimpleScore_.scoreMap[ordinal(mode)] += collectedSimpleScore.score(mode);
        }
    }

    const CollectedCoef& collectedCoef() const { return collectedCoef_; }
    const CollectedSimpleScore& collectedScore() const { return collectedSimpleScore_; }
    const EvaluationParameterMap& evaluationParameterMap() const { return paramMap_; }

    void setEstimatedRensaScore(int s) { estimatedRensaScore_ = s; }
    int estimatedRensaScore() const { return estimatedRensaScore_; }

    void setBookName(const std::string&) {}
    void setPuyosToComplement(const ColumnPuyoList&) {}

private:
    const EvaluationParameterMap& paramMap_;
    CollectedCoef collectedCoef_;
    CollectedSimpleScore collectedSimpleScore_;
    int estimatedRensaScore_ = 0;
};

// This collector collects all features.
class FeatureScoreCollector {
public:
    typedef CollectedFeatureScore CollectedScore;

    FeatureScoreCollector(const EvaluationParameterMap& paramMap) : paramMap_(paramMap) {}

    void setMode(EvaluationMode mode) {
        for (const auto& m : ALL_EVALUATION_MODES)
            collectedCoef_.coefMap[ordinal(m)] = 0.0;
        collectedCoef_.coefMap[ordinal(mode)] = 1.0;
    }

    void addScore(EvaluationFeatureKey key, double v)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            collectedFeatureScore_.collectedSimpleScore.scoreMap[ordinal(mode)] += paramMap_.parameter(mode).score(key, v);
        }
        collectedFeatureScore_.collectedFeatures[key] += v;
    }

    void addScore(EvaluationSparseFeatureKey key, int idx, int n)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            collectedFeatureScore_.collectedSimpleScore.scoreMap[ordinal(mode)] += paramMap_.parameter(mode).score(key, idx, n);
        }
        for (int i = 0; i < n; ++i)
            collectedFeatureScore_.collectedSparseFeatures[key].push_back(idx);
    }

    void merge(const CollectedFeatureScore& cfs)
    {
        for (const auto& mode : ALL_EVALUATION_MODES) {
            collectedFeatureScore_.collectedSimpleScore.scoreMap[ordinal(mode)] += cfs.collectedSimpleScore.score(mode);
        }

        for (const auto& entry : cfs.collectedFeatures) {
            collectedFeatureScore_.collectedFeatures[entry.first] = entry.second;
        }
        for (const auto& entry : cfs.collectedSparseFeatures) {
            collectedFeatureScore_.collectedSparseFeatures[entry.first].insert(
                collectedFeatureScore_.collectedSparseFeatures[entry.first].end(),
                entry.second.begin(),
                entry.second.end());
        }
    }

    void setBookName(const std::string& bookName) { collectedFeatureScore_.bookName = bookName; }
    const std::string& bookName() const { return collectedFeatureScore_.bookName; }

    const CollectedCoef& collectedCoef() const { return collectedCoef_; }
    const CollectedFeatureScore& collectedScore() const { return collectedFeatureScore_; }
    const EvaluationParameterMap& evaluationParameterMap() const { return paramMap_; }

    void setEstimatedRensaScore(int s) { estimatedRensaScore_ = s; }
    int estimatedRensaScore() const { return estimatedRensaScore_; }

    void setPuyosToComplement(const ColumnPuyoList& cpl) { collectedFeatureScore_.puyosToComplement = cpl; }
private:
    const EvaluationParameterMap& paramMap_;
    CollectedCoef collectedCoef_;
    CollectedFeatureScore collectedFeatureScore_;
    int estimatedRensaScore_ = 0;
};

#endif
