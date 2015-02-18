#ifndef FEATURE_PARAMETER_H_
#define FEATURE_PARAMETER_H_

#include <string>
#include <vector>

#include <glog/logging.h>
#include <toml/toml.h>

#include "base/base.h"
#include "evaluation_feature.h"

enum class EvaluationMode {
    DEFAULT,
    EARLY,
    MIDDLE,
    LATE,
};
const EvaluationMode ALL_EVALUATION_MODES[] = {
    EvaluationMode::DEFAULT,
    EvaluationMode::EARLY,
    EvaluationMode::MIDDLE,
    EvaluationMode::LATE,
};
inline int ordinal(EvaluationMode mode) { return static_cast<int>(mode); }
std::string toString(EvaluationMode);

class EvaluationParameter {
public:
    EvaluationParameter();
    explicit EvaluationParameter(const toml::Value&);

    double score(EvaluationFeatureKey key, double value) const { return coef_[key] * value; }
    double score(EvaluationSparseFeatureKey key, int idx, int n) const { return sparseCoef_[key][idx] * n; }

    double getValue(EvaluationFeatureKey key) const { return coef_[key]; }
    std::vector<double> getValues(EvaluationSparseFeatureKey key) const { return sparseCoef_[key]; }
    double getValue(EvaluationSparseFeatureKey key, int index) const { return sparseCoef_[key][index]; }

    void setValue(EvaluationFeatureKey key, double value);
    void addValue(EvaluationFeatureKey key, double value);

    void setValues(EvaluationSparseFeatureKey key, const std::vector<double>& values);
    void setValue(EvaluationSparseFeatureKey key, int index, double value);
    void addValue(EvaluationSparseFeatureKey key, int idx, double value);

    bool isChanged(EvaluationFeatureKey key) const { return coefChanged_[key]; }
    bool isChanged(EvaluationSparseFeatureKey key) const { return sparseCoefChanged_[key]; }

    // Copies the parameter, and changed flag will be cleared.
    void setDefault(const EvaluationParameter&);

    std::string toString() const;

    toml::Value toTomlValue() const;
    bool loadValue(const toml::Value&);

    void removeNontokopuyoParameter();

    friend bool operator==(const EvaluationParameter&, const EvaluationParameter&);

private:
    void setChanged(EvaluationFeatureKey key, bool flag) { coefChanged_[key] = flag; }
    void setChanged(EvaluationSparseFeatureKey key, bool flag) { sparseCoefChanged_[key] = flag; }

    std::vector<double> coef_;
    std::vector<std::vector<double>> sparseCoef_;
    std::vector<bool> coefChanged_;
    std::vector<bool> sparseCoefChanged_;
};

class EvaluationParameterMap {
public:
    EvaluationParameterMap() : map_(ARRAY_SIZE(ALL_EVALUATION_MODES)) {}

    EvaluationParameter* mutableDefaultParameter() { return mutableParameter(EvaluationMode::DEFAULT); }
    const EvaluationParameter& defaultParameter() const { return parameter(EvaluationMode::DEFAULT); }

    EvaluationParameter* mutableParameter(EvaluationMode mode) { return &map_[ordinal(mode)]; }
    const EvaluationParameter& parameter(EvaluationMode mode) const { return map_[ordinal(mode)]; }

    std::string toString() const;

    bool load(const std::string& filename);
    bool save(const std::string& filename) const;

    // For interactive UI.
    void removeNontokopuyoParameter();

private:
    std::vector<EvaluationParameter> map_;
};

#endif
