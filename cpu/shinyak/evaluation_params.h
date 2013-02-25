#ifndef __EVALUATION_PARAMS_H_
#define __EVALUATION_PARAMS_H_

#include <vector>
#include "evaluation_feature.h"

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
