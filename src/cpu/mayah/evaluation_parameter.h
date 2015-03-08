#ifndef FEATURE_PARAMETER_H_
#define FEATURE_PARAMETER_H_

#include <array>
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
    ENEMY_HAS_ZENKESHI,
};

const EvaluationMode ALL_EVALUATION_MODES[] = {
    EvaluationMode::DEFAULT,
    EvaluationMode::EARLY,
    EvaluationMode::MIDDLE,
    EvaluationMode::LATE,
    EvaluationMode::ENEMY_HAS_ZENKESHI,
};
inline int ordinal(EvaluationMode mode) { return static_cast<int>(mode); }
std::string toString(EvaluationMode);

class EvaluationParameter {
public:
    explicit EvaluationParameter(const EvaluationParameter* parent = nullptr);
    EvaluationParameter(const EvaluationParameter* parent, const toml::Value&);

    double score(EvaluationFeatureKey key, double value) const;
    double score(EvaluationSparseFeatureKey key, int idx, int n) const;

    double getValue(EvaluationFeatureKey key) const;
    const std::vector<double>& getValues(EvaluationSparseFeatureKey key) const;
    double getValue(EvaluationSparseFeatureKey key, int index) const;

    void setValue(EvaluationFeatureKey key, double value);
    void addValue(EvaluationFeatureKey key, double value);

    void setValues(EvaluationSparseFeatureKey key, const std::vector<double>& values);
    void setValue(EvaluationSparseFeatureKey key, int index, double value);
    void addValue(EvaluationSparseFeatureKey key, int idx, double value);

    bool hasValue(EvaluationFeatureKey key) const { return coefChanged_[key]; }
    bool hasValue(EvaluationSparseFeatureKey key) const { return sparseCoefChanged_[key]; }

    std::string toString() const;

    toml::Value toTomlValue() const;
    bool loadValue(const toml::Value&);

    void removeNontokopuyoParameter();

    friend bool operator==(const EvaluationParameter&, const EvaluationParameter&);

private:
    void clear();
    void setChanged(EvaluationFeatureKey key, bool flag) { coefChanged_[key] = flag; }
    void setChanged(EvaluationSparseFeatureKey key, bool flag) { sparseCoefChanged_[key] = flag; }

    const EvaluationParameter* parent_;
    std::vector<double> coef_;
    std::vector<std::vector<double>> sparseCoef_;
    std::vector<bool> coefChanged_;
    std::vector<bool> sparseCoefChanged_;
};

inline double EvaluationParameter::score(EvaluationFeatureKey key, double value) const
{
    if (hasValue(key) || !parent_)
        return coef_[key] * value;
    return parent_->score(key, value);
}

inline double EvaluationParameter::score(EvaluationSparseFeatureKey key, int idx, int n) const
{
    if (hasValue(key) || !parent_)
        return sparseCoef_[key][idx] * n;
    return parent_->score(key, idx, n);
}

inline double EvaluationParameter::getValue(EvaluationFeatureKey key) const
{
    if (hasValue(key) || !parent_)
        return coef_[key];
    return parent_->getValue(key);
}

inline const std::vector<double>& EvaluationParameter::getValues(EvaluationSparseFeatureKey key) const
{
    if (hasValue(key) || !parent_)
        return sparseCoef_[key];
    return parent_->getValues(key);
}

inline double EvaluationParameter::getValue(EvaluationSparseFeatureKey key, int index) const
{
    if (hasValue(key) || !parent_)
        return sparseCoef_[key][index];
    return parent_->getValue(key, index);
}

// ----------------------------------------------------------------------

class EvaluationParameterMap {
public:
    EvaluationParameterMap();
    ~EvaluationParameterMap();

    EvaluationParameterMap(const EvaluationParameterMap&);
    EvaluationParameterMap& operator=(const EvaluationParameterMap&);

    EvaluationParameter* mutableDefaultParameter() { return mutableParameter(EvaluationMode::DEFAULT); }
    const EvaluationParameter& defaultParameter() const { return parameter(EvaluationMode::DEFAULT); }

    EvaluationParameter* mutableParameter(EvaluationMode mode) { return map_[ordinal(mode)].get(); }
    const EvaluationParameter& parameter(EvaluationMode mode) const { return *map_[ordinal(mode)]; }

    std::string toString() const;

    toml::Value toTomlValue() const;
    bool loadValue(const toml::Value&);

    bool load(const std::string& filename);
    bool save(const std::string& filename) const;

    // For interactive UI.
    void removeNontokopuyoParameter();

private:
    std::array<std::unique_ptr<EvaluationParameter>, ARRAY_SIZE(ALL_EVALUATION_MODES)> map_;
};

#endif
