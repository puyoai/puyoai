#include "evaluation_parameter.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include <gflags/gflags.h>

#include "core/core_field.h"

using namespace std;

string toString(EvaluationMode mode)
{
    switch (mode) {
    case EvaluationMode::DEFAULT: return "";
    case EvaluationMode::EARLY: return "early";
    case EvaluationMode::EARLY_MIDDLE: return "early_middle";
    case EvaluationMode::MIDDLE: return "middle";
    case EvaluationMode::LATE: return "late";
    case EvaluationMode::ENEMY_HAS_ZENKESHI: return "enemy_has_zenkeshi";
    default:
        CHECK(false) << static_cast<int>(mode);
    }
}

EvaluationParameter::EvaluationParameter(const EvaluationParameter* parent) :
    parent_(parent),
    coef_(EvaluationFeature::all().size()),
    sparseCoef_(EvaluationSparseFeature::all().size()),
    coefChanged_(EvaluationFeature::all().size()),
    sparseCoefChanged_(EvaluationSparseFeature::all().size())
{
    for (const EvaluationSparseFeature& feature : EvaluationSparseFeature::all()) {
        sparseCoef_[feature.key()].resize(feature.size());
    }
}

EvaluationParameter::EvaluationParameter(const EvaluationParameter* parent, const toml::Value& value) :
    EvaluationParameter::EvaluationParameter(parent)
{
    CHECK(loadValue(value));
}

void EvaluationParameter::setValue(EvaluationFeatureKey key, double value)
{
    coef_[key] = value;
    coefChanged_[key] = true;
}

void EvaluationParameter::addValue(EvaluationFeatureKey key, double value)
{
    coef_[key] += value;
    coefChanged_[key] = true;
}

void EvaluationParameter::setValues(EvaluationSparseFeatureKey key, const std::vector<double>& values)
{
    sparseCoef_[key] = values;
    sparseCoefChanged_[key] = true;
}

void EvaluationParameter::setValue(EvaluationSparseFeatureKey key, int index, double value)
{
   sparseCoef_[key][index] = value;
   sparseCoefChanged_[key] = true;
}

void EvaluationParameter::addValue(EvaluationSparseFeatureKey key, int idx, double value)
{
    DCHECK(0 <= idx && static_cast<size_t>(idx) < sparseCoef_[key].size())
        << "key=" << EvaluationSparseFeature::toFeature(key).str()
        << " idx=" << idx
        << " size=" << sparseCoef_[key].size();
    sparseCoef_[key][idx] += value;
    sparseCoefChanged_[key] = true;
}

toml::Value EvaluationParameter::toTomlValue() const
{
    toml::Value v;

    for (const auto& ef : EvaluationFeature::all()) {
        if (!hasValue(ef.key()))
            continue;
        v.set(ef.str(), getValue(ef.key()));
    }

    for (const auto& ef : EvaluationSparseFeature::all()) {
        if (!hasValue(ef.key()))
            continue;

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
    clear();

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

        setValue(ef.key(), v->asNumber());
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

        vector<double> vs(sparseCoef_[ef.key()].size());
        const toml::Array& ary = v->as<toml::Array>();

        if (ary.size() != vs.size()) {
            LOG(ERROR) << ef.key() << " should have size " << vs.size()
                       << ", but configuration has size " << ary.size();
            return false;
        }

        for (size_t i = 0; i < ary.size(); ++i) {
            if (!ary[i].isNumber()) {
                LOG(ERROR) << ef.key() << "[" << i << "] is not a number.";
                return false;
            }

            vs[i] = ary[i].asNumber();
        }

        setValues(ef.key(), vs);
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
        if (!hasValue(ef.key()))
            continue;
        ss << ef.str() << " = " << coef_[ef.key()] << endl;
    }

    for (const EvaluationSparseFeature& ef : EvaluationSparseFeature::all()) {
        if (!hasValue(ef.key()))
            continue;
        ss << ef.str() << " =";
        for (size_t i = 0; i < ef.size(); ++i) {
            ss << ' ' << sparseCoef_[ef.key()][i];
        }
        ss << endl;
    }

    return ss.str();
}

void EvaluationParameter::removeNontokopuyoParameter()
{
    for (const auto& ef : EvaluationFeature::all()) {
        if (ef.shouldIgnore()) {
            setValue(ef.key(), 0);
            setChanged(ef.key(), false);
        }
    }

    for (const auto& ef : EvaluationSparseFeature::all()) {
        if (ef.shouldIgnore()) {
            for (size_t i = 0; i < ef.size(); ++i) {
                setValue(ef.key(), i, 0);
                setChanged(ef.key(), false);
            }
        }
    }
}

void EvaluationParameter::clear()
{
    for (const auto& ef : EvaluationFeature::all()) {
        coef_[ef.key()] = 0.0;
        setChanged(ef.key(), false);
    }
    for (const auto& ef : EvaluationSparseFeature::all()) {
        std::fill(sparseCoef_[ef.key()].begin(),
                  sparseCoef_[ef.key()].end(),
                  0.0);
        setChanged(ef.key(), false);
    }
}

bool operator==(const EvaluationParameter& lhs, const EvaluationParameter& rhs)
{
    return lhs.coef_ == rhs.coef_ && lhs.sparseCoef_ == rhs.sparseCoef_;
}

EvaluationParameterMap::EvaluationParameterMap()
{
    for (size_t i = 0; i < map_.size(); ++i) {
        if (i == 0)
            map_[i].reset(new EvaluationParameter(nullptr));
        else
            map_[i].reset(new EvaluationParameter(map_[0].get()));
    }
}

EvaluationParameterMap::~EvaluationParameterMap()
{
}

EvaluationParameterMap::EvaluationParameterMap(const EvaluationParameterMap& map) :
    EvaluationParameterMap()
{
    loadValue(map.toTomlValue());
}

EvaluationParameterMap& EvaluationParameterMap::operator=(const EvaluationParameterMap& map)
{
    loadValue(map.toTomlValue());
    return *this;
}

bool EvaluationParameterMap::load(const string& filename)
{
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

    return loadValue(value);
}

bool EvaluationParameterMap::loadValue(const toml::Value& v)
{
    toml::Value value(v);

    toml::Value modeValues;
    if (toml::Value* modeValue = value.find("mode")) {
        modeValues = std::move(*modeValue);
        value.erase("mode");
    }

    mutableDefaultParameter()->loadValue(value);
    for (auto mode : ALL_EVALUATION_MODES) {
        if (mode == EvaluationMode::DEFAULT)
            continue;
        if (toml::Value* v = modeValues.find(::toString(mode))) {
            mutableParameter(mode)->loadValue(*v);
            modeValues.erase(::toString(mode));
        }
    }

    // check modeValues is empty.
    CHECK(modeValues.empty()) << modeValues;

    return true;
}

bool EvaluationParameterMap::save(const string& filename) const
{
    toml::Value value = toTomlValue();

    try {
        ofstream ofs(filename, ios::out | ios::trunc);
        value.write(&ofs);
    } catch (std::exception& e) {
        LOG(WARNING) << "EvaluationParameter::save failed: " << e.what();
        return false;
    }

    return true;
}

toml::Value EvaluationParameterMap::toTomlValue() const
{
    toml::Value value = defaultParameter().toTomlValue();
    for (auto mode : ALL_EVALUATION_MODES) {
        if (mode == EvaluationMode::DEFAULT)
            continue;

        toml::Value* v = value.ensureTable(string("mode.") + ::toString(mode));
        *v = parameter(mode).toTomlValue();
    }

    return value;
}

string EvaluationParameterMap::toString() const
{
    toml::Value value = toTomlValue();

    stringstream ss;
    ss << value;
    return ss.str();
}

void EvaluationParameterMap::removeNontokopuyoParameter()
{
    for (auto mode : ALL_EVALUATION_MODES) {
        mutableParameter(mode)->removeNontokopuyoParameter();
    }
}
