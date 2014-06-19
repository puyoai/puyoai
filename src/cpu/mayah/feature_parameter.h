#ifndef FEATURE_PARAMETER_H_
#define FEATURE_PARAMETER_H_

#include <string>
#include <vector>
#include <glog/logging.h>

#include "evaluation_feature_key.h"

class FeatureParameter {
public:
    explicit FeatureParameter(const char* filename);

    double score(EvaluationFeatureKey key, double value) const { return coef_[key] * value; }
    double score(EvaluationSparseFeatureKey key, int idx) const { return sparseCoef_[key][idx]; }

    double getValue(EvaluationFeatureKey key) const { return coef_[key]; }
    void setValue(EvaluationFeatureKey key, double value) { coef_[key] = value; }
    void addValue(EvaluationFeatureKey key, double value) { coef_[key] += value; }

    double getValue(EvaluationSparseFeatureKey key, int idx) const { return sparseCoef_[key][idx]; }
    void setValue(EvaluationSparseFeatureKey key, int idx, double value) { sparseCoef_[key][idx] = value; }
    void setValue(EvaluationSparseFeatureKey key, const std::vector<double>& values) { sparseCoef_[key] = values; }
    void addValue(EvaluationSparseFeatureKey key, int idx, double value)
    {
        CHECK(0 <= idx && static_cast<size_t>(idx) < sparseCoef_[key].size())
            << "key=" << ::toString(key)
            << " idx=" << idx
            << " size=" << sparseCoef_[key].size();
        sparseCoef_[key][idx] += value;
    }

    std::string toString() const;

    bool save(const char* filename);
    bool load(const char* filename);

    friend bool operator==(const FeatureParameter&, const FeatureParameter&);

private:
    std::vector<double> coef_;
    std::vector<std::vector<double> > sparseCoef_;
};

#endif
