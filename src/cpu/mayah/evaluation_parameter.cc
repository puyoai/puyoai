#include "evaluation_parameter.h"

#include <map>
#include <set>
#include <sstream>
#include <vector>

#include <gflags/gflags.h>

#include "core/core_field.h"

using namespace std;

EvaluationParameter::EvaluationParameter() :
    coef_(EvaluationFeature::all().size()),
    sparseCoef_(EvaluationSparseFeature::all().size())
{
    for (const EvaluationSparseFeature& feature : EvaluationSparseFeature::all()) {
        sparseCoef_[feature.key()].resize(feature.size());
    }
}

EvaluationParameter::EvaluationParameter(const toml::Value& value) :
    EvaluationParameter::EvaluationParameter()
{
    CHECK(loadValue(value));
}

toml::Value EvaluationParameter::toTomlValue() const
{
    toml::Value v;

    for (const auto& ef : EvaluationFeature::all()) {
        v.set(ef.str(), getValue(ef.key()));
    }

    for (const auto& ef : EvaluationSparseFeature::all()) {
        toml::Value vs;
        for (size_t i = 0; i < ef.size(); ++i) {
            vs.push(getValue(ef.key(), i));
        }

        v.set(ef.str(), vs);
    }

    return v;
}

bool EvaluationParameter::loadValue(const toml::Value& value)
{
    set<string> keys;
    for (const auto& entry : value.as<toml::Table>()) {
        keys.insert(entry.first);
    }

    for (const EvaluationFeature& ef : EvaluationFeature::all()) {
        const toml::Value* v = value.find(ef.str());
        if (!v)
            continue;

        if (!v->isNumber()) {
            LOG(ERROR) << ef.key() << " is not a number";
            return false;
        }

        coef_[ef.key()] = v->asNumber();
        keys.erase(ef.str());
    }

    for (const EvaluationSparseFeature& ef : EvaluationSparseFeature::all()) {
        const toml::Value* v = value.find(ef.str());
        if (!v)
            continue;

        if (!v->is<toml::Array>()) {
            LOG(ERROR) << ef.key() << " is not an array";
            return false;
        }

        vector<double>* vs = &sparseCoef_[ef.key()];
        const toml::Array& ary = v->as<toml::Array>();

        if (ary.size() != vs->size()) {
            LOG(ERROR) << ef.key() << " should have size " << vs->size()
                       << ", but configuration has size " << ary.size();
            return false;
        }

        for (size_t i = 0; i < ary.size(); ++i) {
            if (!ary[i].isNumber()) {
                LOG(ERROR) << ef.key() << "[" << i << "] is not a number.";
                return false;
            }

            (*vs)[i] = ary[i].asNumber();
        }

        keys.erase(ef.str());
    }

    if (!keys.empty()) {
        stringstream ss;
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

string EvaluationParameter::toString() const
{
    ostringstream ss;

    for (const EvaluationFeature& ef : EvaluationFeature::all()) {
        ss << ef.str() << " = " << coef_[ef.key()] << endl;
    }

    for (const EvaluationSparseFeature& ef : EvaluationSparseFeature::all()) {
        ss << ef.str() << " =";
        for (size_t i = 0; i < ef.size(); ++i) {
            ss << ' ' << sparseCoef_[ef.key()][i];
        }
        ss << endl;
    }

    return ss.str();
}

bool operator==(const EvaluationParameter& lhs, const EvaluationParameter& rhs)
{
    return lhs.coef_ == rhs.coef_ && lhs.sparseCoef_ == rhs.sparseCoef_;
}
