#ifndef CPU_MAYAH_FEATURE_PARAMETER_H_
#define CPU_MAYAH_FEATURE_PARAMETER_H_

#include <algorithm>
#include <array>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <glog/logging.h>
#include <toml/toml.h>

#include "base/base.h"
#include "evaluation_feature.h"
#include "evaluation_mode.h"

template<typename FeatureSet>
class EvaluationParameter {
public:
    typedef typename FeatureSet::FeatureKey FeatureKey;
    typedef typename FeatureSet::SparseFeatureKey SparseFeatureKey;

    EvaluationParameter() :
        param_(FeatureSet::features().size()),
        hasParam_(FeatureSet::features().size()),
        sparseParam_(FeatureSet::sparseFeatures().size()),
        hasSparseParam_(FeatureSet::sparseFeatures().size())
    {
        for (const auto& feature : FeatureSet::sparseFeatures()) {
            sparseParam_[feature.key()].resize(feature.size());
        }
    }

    double param(FeatureKey key) const { return param_[key]; }
    double param(SparseFeatureKey key, int idx) const { return sparseParam_[key][idx]; }
    bool hasParam(FeatureKey key) const { return hasParam_[key]; }
    bool hasParam(SparseFeatureKey key) const { return hasSparseParam_[key]; }

    void setParam(FeatureKey key, double value)
    {
        param_[key] = value;
        hasParam_[key] = true;
    }

    void setParam(SparseFeatureKey key, int index, double value)
    {
        sparseParam_[key][index] = value;
        hasSparseParam_[key] = true;
    }

    void removeNontokopuyoParameter()
    {
        for (const auto& ef : FeatureSet::features()) {
            if (ef.shouldIgnore()) {
                param_[ef.key()] = 0;
                hasParam_[ef.key()] = false;
            }
        }

        for (const auto& ef : FeatureSet::sparseFeatures()) {
            if (ef.shouldIgnore()) {
                for (size_t i = 0; i < ef.size(); ++i) {
                    sparseParam_[ef.key()][i] = 0;
                }
                hasSparseParam_[ef.key()] = false;
            }
        }
    }

    std::string toString() const
    {
        std::stringstream ss;
        ss << toTomlValue();
        return ss.str();
    }

    toml::Value toTomlValue() const
    {
        toml::Value v;

        for (const auto& ef : FeatureSet::features()) {
            if (!hasParam(ef.key()))
                continue;
            v.set(ef.name(), param(ef.key()));
        }

        for (const auto& ef : FeatureSet::sparseFeatures()) {
            if (!hasParam(ef.key()))
                continue;

            toml::Value vs;
            for (size_t i = 0; i < ef.size(); ++i) {
                vs.push(param(ef.key(), i));
            }
            v.set(ef.name(), vs);
        }

        return v;
    }

    bool loadValue(const toml::Value& value)
    {
        clear();

        std::set<std::string> keys;
        for (const auto& entry : value.as<toml::Table>()) {
            keys.insert(entry.first);
        }

        for (const auto& ef : FeatureSet::features()) {
            const toml::Value* v = value.find(ef.name());
            if (!v)
                continue;

            if (!v->isNumber()) {
                LOG(ERROR) << ef.key() << " is not a number";
                return false;
            }

            setParam(ef.key(), v->asNumber());
            keys.erase(ef.name());
        }

        for (const auto& ef : FeatureSet::sparseFeatures()) {
            const toml::Value* v = value.find(ef.name());
            if (!v)
                continue;

            if (!v->is<toml::Array>()) {
                LOG(ERROR) << ef.key() << " is not an array";
                return false;
            }

            const toml::Array& ary = v->as<toml::Array>();

            if (ary.size() != sparseParam_[ef.key()].size()) {
                LOG(ERROR) << ef.key() << " should have size " << sparseParam_[ef.key()].size()
                           << ", but configuration has size " << ary.size();
                return false;
            }

            for (size_t i = 0; i < ary.size(); ++i) {
                if (!ary[i].isNumber()) {
                    LOG(ERROR) << ef.key() << "[" << i << "] is not a number.";
                    return false;
                }

                setParam(ef.key(), i, ary[i].asNumber());
            }
            keys.erase(ef.name());
        }

        if (!keys.empty()) {
            std::stringstream ss;
            bool first = true;
            for (const auto& key : keys) {
                if (first) {
                    first = false;
                } else {
                    ss << ",";
                }
                ss << key;
            }
            CHECK(false) << "Unknown feature key is specified: " << ss.str();
        }

        return true;
    }

    void clear()
    {
        std::fill(param_.begin(), param_.end(), 0.0);
        for (auto& p : sparseParam_) {
            std::fill(p.begin(), p.end(), 0.0);
        }
        std::fill(hasParam_.begin(), hasParam_.end(), false);
        std::fill(hasSparseParam_.begin(), hasSparseParam_.end(), false);
    }

protected:
    std::vector<double> param_;
    std::vector<bool> hasParam_;
    std::vector<std::vector<double>> sparseParam_;
    std::vector<bool> hasSparseParam_;
};

typedef EvaluationParameter<EvaluationMoveFeatureSet> EvaluationMoveParameter;
typedef EvaluationParameter<EvaluationRensaFeatureSet> EvaluationRensaParameter;

// ----------------------------------------------------------------------

template<typename Param, typename FeatureSet>
class EvaluationParameterSet {
public:
    typedef typename FeatureSet::FeatureKey FeatureKey;
    typedef typename FeatureSet::SparseFeatureKey SparseFeatureKey;

    double param(EvaluationMode mode, FeatureKey key) const
    {
        if (params_[ordinal(mode)].hasParam(key))
            return params_[ordinal(mode)].param(key);
        return defaultParam_.param(key);
    }

    double param(EvaluationMode mode, SparseFeatureKey key, int idx) const
    {
        if (params_[ordinal(mode)].hasParam(key))
            return params_[ordinal(mode)].param(key, idx);
        return defaultParam_.param(key, idx);
    }

    void setParam(EvaluationMode mode, FeatureKey key, double value)
    {
        params_[ordinal(mode)].setParam(key, value);
    }

    void setParam(EvaluationMode mode, SparseFeatureKey key, int index, double value)
    {
        params_[ordinal(mode)].setParam(key, index, value);
    }

    void setDefault(FeatureKey key, double value)
    {
        defaultParam_.setParam(key, value);
    }

    void setDefault(SparseFeatureKey key, int index, double value)
    {
        defaultParam_.setParam(key, index, value);
    }

    void removeNontokopuyoParameter()
    {
        defaultParam_.removeNontokopuyoParameter();
        for (auto& param : params_) {
            param.removeNontokopuyoParameter();
        }
    }

    void clear()
    {
        defaultParam_.clear();
        for (auto& param : params_) {
            param.clear();
        }
    }

    toml::Value toTomlValue() const
    {
        toml::Value value = defaultParam_.toTomlValue();
        for (const auto& mode : ALL_EVALUATION_MODES) {
            toml::Value* v = value.ensureTable(std::string("mode.") + ::toString(mode));
            *v = params_[ordinal(mode)].toTomlValue();
        }

        return value;
    }

    bool loadValue(const toml::Value& value, const std::string& anotherKey)
    {
        CHECK(!anotherKey.empty()) << "another key should not be empty.";

        // Checks mode does not have unnecessary value.
        if (const toml::Value* v = value.find("mode")) {
            std::set<std::string> keys;
            for (const auto& kv : v->as<toml::Table>()) {
                keys.insert(kv.first);
            }

            for (const auto& mode : ALL_EVALUATION_MODES) {
                keys.erase(::toString(mode));
            }
            keys.erase("default");

            CHECK(keys.empty());
        }

        for (const auto& mode : ALL_EVALUATION_MODES) {
            std::string modeKey = std::string("mode.") + ::toString(mode) + std::string(".") + anotherKey;

            const toml::Value* v = value.find(modeKey);
            if (!v)
                continue;
            if (!params_[ordinal(mode)].loadValue(*v))
                return false;
        }

        {
            std::string defaultKey = std::string("mode.default.") + anotherKey;
            const toml::Value* v = value.find(defaultKey);
            CHECK(v != nullptr) << defaultKey << "was not found.";
            if (!defaultParam_.loadValue(*v))
                return false;
        }

        return true;
    }

private:
    Param defaultParam_;
    std::array<Param, NUM_EVALUATION_MODES> params_;
};

typedef EvaluationParameterSet<EvaluationMoveParameter, EvaluationMoveFeatureSet> EvaluationMoveParameterSet;
typedef EvaluationParameterSet<EvaluationRensaParameter, EvaluationRensaFeatureSet> EvaluationRensaParameterSet;

class EvaluationParameterMap {
public:
    const EvaluationMoveParameterSet& moveParamSet() const { return moveParamSet_; }
    const EvaluationRensaParameterSet& mainRensaParamSet() const { return mainRensaParamSet_; }
    const EvaluationRensaParameterSet& sideRensaParamSet() const { return sideRensaParamSet_; }

    EvaluationMoveParameterSet* mutableMoveParamSet() { return &moveParamSet_; }
    EvaluationRensaParameterSet* mutableMainRensaParamSet() { return &mainRensaParamSet_; }
    EvaluationRensaParameterSet* mutableSideRensaParamSet() { return &sideRensaParamSet_; }

    std::string toString() const;

    toml::Value toTomlValue() const;
    bool loadValue(const toml::Value&);

    bool load(const std::string& filename);
    bool save(const std::string& filename) const;

    // For interactive UI.
    void removeNontokopuyoParameter();

private:
    EvaluationMoveParameterSet moveParamSet_;
    EvaluationRensaParameterSet mainRensaParamSet_;
    EvaluationRensaParameterSet sideRensaParamSet_;
};

#endif // CPU_MAYAH_FEATURE_PARAMETER_H_
