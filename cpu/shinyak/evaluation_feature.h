#ifndef __EVALUATION_FEATURE_H_
#define __EVALUATION_FEATURE_H_

#include <string>
#include <vector>

enum IntegerFeatureParam {
    MAX_CHAINS, // Max chains
    MAX_RENSA_NECESSARY_PUYOS, // Necessary puyos to fire max chains

    THIRD_COLUMN_HEIGHT,

    CONNECTION_1,
    CONNECTION_2,
    CONNECTION_3,
    CONNECTION_4,

    CONNECTION_AFTER_VANISH_1,
    CONNECTION_AFTER_VANISH_2,
    CONNECTION_AFTER_VANISH_3,
    CONNECTION_AFTER_VANISH_4,

    HAND_WIDTH_1,
    HAND_WIDTH_2,
    HAND_WIDTH_3,
    HAND_WIDTH_4,

    TOTAL_FRAMES,

    SIZE_OF_INTEGER_FEATURE_PARAM
};

enum DoubleFeatureParam {
    SUM_OF_HEIGHT_DIFF_FROM_AVERAGE,
    SQUARE_SUM_OF_HEIGHT_DIFF_FROM_AVERAGE,

    EMPTY_AVAILABILITY_00,
    EMPTY_AVAILABILITY_01,
    EMPTY_AVAILABILITY_02,
    EMPTY_AVAILABILITY_11,
    EMPTY_AVAILABILITY_12,
    EMPTY_AVAILABILITY_22,

    SIZE_OF_DOUBLE_FEATURE_PARAM
};

class EvaluationFeature {
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
