#ifndef FEATURE_PARAMETER_H_
#define FEATURE_PARAMETER_H_

#include <string>
#include <vector>

#include <glog/logging.h>
#include <toml/toml.h>

#include "evaluation_feature.h"

class EvaluationParameter {
public:
    EvaluationParameter();
    explicit EvaluationParameter(const std::string& filename);

    double score(EvaluationFeatureKey key, double value) const { return coef_[key] * value; }
    double score(EvaluationSparseFeatureKey key, int idx, int n) const { return sparseCoef_[key][idx] * n; }

    double getValue(EvaluationFeatureKey key) const { return coef_[key]; }
    void setValue(EvaluationFeatureKey key, double value) { coef_[key] = value; }
    void addValue(EvaluationFeatureKey key, double value) { coef_[key] += value; }

    std::vector<double> getValues(EvaluationSparseFeatureKey key) const { return sparseCoef_[key]; }
    void setValues(EvaluationSparseFeatureKey key, const std::vector<double>& values) { sparseCoef_[key] = values; }
    double getValue(EvaluationSparseFeatureKey key, int index) const { return sparseCoef_[key][index]; }
    void setValue(EvaluationSparseFeatureKey key, int index, double value) { sparseCoef_[key][index] = value; }
    void addValue(EvaluationSparseFeatureKey key, int idx, double value)
    {
        CHECK(0 <= idx && static_cast<size_t>(idx) < sparseCoef_[key].size())
            << "key=" << EvaluationSparseFeature::toFeature(key).str()
            << " idx=" << idx
            << " size=" << sparseCoef_[key].size();
        sparseCoef_[key][idx] += value;
    }

    std::string toString() const;

    bool save(const std::string& filename);
    bool load(const std::string& filename);

    toml::Value toTomlValue() const;
    bool loadValue(const toml::Value&);

    friend bool operator==(const EvaluationParameter&, const EvaluationParameter&);

private:
    std::vector<double> coef_;
    std::vector<std::vector<double>> sparseCoef_;
};

#endif
