#ifndef __EVALUATION_FEATURE_H_
#define __EVALUATION_FEATURE_H_

#include <string>
#include <vector>

class EvaluationParams;

class EvaluationFeature {
public:
    enum FeatureParam {
#define DEFINE_PARAM(NAME) NAME,
#define DEFINE_RANGE_PARAM(NAME, maxValue, asc) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
        SIZE_OF_FEATURE_PARAM
    };

    enum RangeFeatureParam {
#define DEFINE_PARAM(NAME) /* ignored */
#define DEFINE_RANGE_PARAM(NAME, maxValue, asc) NAME,
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
        SIZE_OF_RANGE_FEATURE_PARAM
    };

public:
    EvaluationFeature() :
        m_features(SIZE_OF_FEATURE_PARAM),
        m_rangeFeatures(SIZE_OF_RANGE_FEATURE_PARAM)
    {
    }

public:
    void set(FeatureParam param, double value) { m_features[param] = value; }
    void add(FeatureParam param, double value) { m_features[param] += value; }
    double get(FeatureParam param) const { return m_features[param]; }

    void set(RangeFeatureParam param, int value) { m_rangeFeatures[param] = value; }
    int get(RangeFeatureParam param) const { return m_rangeFeatures[param]; }

public:
    std::string toString() const;

private:
    double chainScore() const;

private:
    // TODO(mayah): We would like to use std::array instead.
    std::vector<double> m_features;
    std::vector<int> m_rangeFeatures;
};

class EvaluationParams {
public:
    EvaluationParams();

    void set(EvaluationFeature::FeatureParam param, double value) { m_featuresCoef[param] = value; }
    void set(EvaluationFeature::RangeFeatureParam param, int i, double value) { m_rangeFeaturesCoef[param][i] = value; }
    void set(EvaluationFeature::RangeFeatureParam param, const std::vector<double>& values) { m_rangeFeaturesCoef[param] = values; }

    double get(EvaluationFeature::FeatureParam param) const { return m_featuresCoef[param]; }
    double get(EvaluationFeature::RangeFeatureParam param, int index) const { return m_rangeFeaturesCoef[param][index]; }

public:
    double calculateHandWidthScore(int numFirstCells, int numSecondCells, int numThirdCells, int numFourthCells) const;
    double calculateScore(const EvaluationFeature&) const;

private:
    void initialize();
    
    std::vector<double> m_featuresCoef;
    std::vector<std::vector<double> > m_rangeFeaturesCoef;
};

#endif
