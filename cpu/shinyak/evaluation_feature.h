#ifndef __EVALUATION_FEATURE_H_
#define __EVALUATION_FEATURE_H_

#include <string>
#include <vector>

enum FeatureParam {
    MAX_CHAINS,
    MAX_RENSA_NECESSARY_PUYOS,
    SIZE_OF_FEATURE_PARAM
};

class EvaluationFeature {
public:

public:
    EvaluationFeature() :
        m_features(SIZE_OF_FEATURE_PARAM)
    {
    }

public:
    void set(FeatureParam param, int value) { m_features[param] = value; }
    int get(FeatureParam param) const { return m_features[param]; }

public:
    std::string toString() const;
    double calculateScore() const;
private:
    std::vector<int> m_features;
};

#endif
