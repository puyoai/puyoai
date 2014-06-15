#ifndef EVALUATION_FEATURE_H_
#define EVALUATION_FEATURE_H_

#include <string>
#include <vector>
#include <glog/logging.h>

#define USE_CONNECTION_FEATURE 0
#define USE_EMPTY_AVAILABILITY_FEATURE 0
#define USE_HAND_WIDTH_FEATURE 0
#define USE_THIRD_COLUMN_HEIGHT_FEATURE 0

enum EvaluationFeatureKey {
#define DEFINE_PARAM(NAME) NAME,
#define DEFINE_SPARSE_PARAM(NAME, numValue) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
    SIZE_OF_EVALUATION_FEATURE_KEY
};

enum EvaluationSparseFeatureKey {
#define DEFINE_PARAM(NAME) /* ignored */
#define DEFINE_SPARSE_PARAM(NAME, numValue) NAME,
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
    SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY
};

inline EvaluationFeatureKey toEvaluationFeatureKey(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_EVALUATION_FEATURE_KEY);
    return static_cast<EvaluationFeatureKey>(ith);
}

inline EvaluationSparseFeatureKey toEvaluationSparseFeatureKey(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY);
    return static_cast<EvaluationSparseFeatureKey>(ith);
}

class EvaluationFeature {
public:
    // If filename is NULL, all the parameters will be 0.
    explicit EvaluationFeature(const char* filename);

    double score(EvaluationFeatureKey key, double value) const { return featuresCoef_[key] * value; }
    double score(EvaluationSparseFeatureKey key, int idx) const { return sparseFeaturesCoef_[key][idx]; }

    double getValue(EvaluationFeatureKey key) const { return featuresCoef_[key]; }
    void setValue(EvaluationFeatureKey key, double value) { featuresCoef_[key] = value; }
    void addValue(EvaluationFeatureKey key, double value) { featuresCoef_[key] += value; }

    double getValue(EvaluationSparseFeatureKey key, int idx) const { return sparseFeaturesCoef_[key][idx]; }
    void setValue(EvaluationSparseFeatureKey key, int idx, double value) { sparseFeaturesCoef_[key][idx] = value; }
    void setValue(EvaluationSparseFeatureKey key, const std::vector<double>& values) { sparseFeaturesCoef_[key] = values; }
    void addValue(EvaluationSparseFeatureKey key, int idx, double value) { sparseFeaturesCoef_[key][idx] += value; }

    std::string toString() const;

    bool save(const char* filename);
    bool load(const char* filename);

    friend bool operator==(const EvaluationFeature&, const EvaluationFeature&);

private:
    std::vector<double> featuresCoef_;
    std::vector<std::vector<double> > sparseFeaturesCoef_;
};

#endif
