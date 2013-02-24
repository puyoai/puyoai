#ifndef __EVALUATION_FEATURE_H_
#define __EVALUATION_FEATURE_H_

#include <string>
#include <vector>

class EvaluationFeature {
public:
    enum IntegerFeatureParam {
#define DEFINE_INT_RANGE_PARAM(NAME, minValue, maxValue, asc) NAME,
#define DEFINE_INT_SINGLE_PARAM(NAME, asc) NAME,
#include "evaluation_feature_int.tab"
#undef DEFINE_INT_SINGLE_PARAM
#undef DEFINE_INT_RANGE_PARAM
        SIZE_OF_INTEGER_FEATURE_PARAM
    };

enum DoubleFeatureParam {
#define DEFINE_DOUBLE_PARAM(NAME) NAME,
#include "evaluation_feature_double.tab"
#undef DEFINE_DOUBLE_PARAMS
    SIZE_OF_DOUBLE_FEATURE_PARAM
};

public:
    EvaluationFeature() :
        m_integerFeatures(SIZE_OF_INTEGER_FEATURE_PARAM),
        m_doubleFeatures(SIZE_OF_DOUBLE_FEATURE_PARAM)
    {
    }

public:
    void set(IntegerFeatureParam param, int value) { m_integerFeatures[param] = value; }
    void add(IntegerFeatureParam param, int value) { m_integerFeatures[param] += value; }
    int get(IntegerFeatureParam param) const { return m_integerFeatures[param]; }

    void set(DoubleFeatureParam param, double value) { m_doubleFeatures[param] = value; }
    void add(DoubleFeatureParam param, double value) { m_doubleFeatures[param] += value; }
    double get(DoubleFeatureParam param) const { return m_doubleFeatures[param]; }

public:
    static double calculateHandWidthScore(int numFirstCells, int numSecondCells, int numThirdCells, int numFourthCells);
    double calculateScore() const;
    std::string toString() const;

private:
    double chainScore() const;

private:
    std::vector<int> m_integerFeatures;
    std::vector<double> m_doubleFeatures;
};

#endif
