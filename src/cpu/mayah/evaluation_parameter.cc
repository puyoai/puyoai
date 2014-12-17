#include "evaluation_parameter.h"

#include <fstream>
#include <map>
#include <sstream>
#include <vector>

#include <gflags/gflags.h>
#include <toml/toml.h>

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

EvaluationParameter::EvaluationParameter(const string& filename) :
    coef_(EvaluationFeature::all().size()),
    sparseCoef_(EvaluationSparseFeature::all().size())
{
    CHECK(load(filename));
}

bool EvaluationParameter::save(const string& filename)
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

    try {
        ofstream ofs(filename, ios::out | ios::trunc);
        v.write(&ofs);
        return true;
    } catch (std::exception& e) {
        LOG(WARNING) << "EvaluationParameter::save failed: " << e.what();
        return false;
    }
}

bool EvaluationParameter::load(const string& filename)
{
    for (const EvaluationSparseFeature& feature : EvaluationSparseFeature::all()) {
        sparseCoef_[feature.key()].resize(feature.size());
    }

    toml::Value value;
    try {
        ifstream ifs(filename, ios::in);
        toml::Parser parser(ifs);
        value = parser.parse();
        if (!value.valid()) {
            LOG(ERROR) << parser.errorReason();
            return false;
        }
    } catch (std::exception& e) {
        LOG(WARNING) << "EvaluationParameter::load failed: " << e.what();
        return false;
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
        value.erase(ef.str());
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

        value.erase(ef.str());
    }

    if (!value.empty()) {
        stringstream ss;
        bool first = true;
        for (const auto& entry : value.as<toml::Table>()) {
            if (first) {
                first = false;
            } else {
                ss << ",";
            }
            ss << entry.first;
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
