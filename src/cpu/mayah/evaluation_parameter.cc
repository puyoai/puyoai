#include "evaluation_parameter.h"

#include <glog/logging.h>
#include <toml/toml.h>

#include <algorithm>
#include <fstream>
#include <set>
#include <vector>

#include <cstddef>
#include <exception>
#include <utility>

#include "cpu/mayah/evaluation_feature.h"

using namespace std;

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
        LOG(WARNING) << "EvaluationParameterMap::load failed: " << e.what();
        return false;
    }

    return loadValue(value);
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

bool EvaluationParameterMap::loadValue(const toml::Value& v)
{
    // Check v has only |mode| key
    CHECK(v.is<toml::Table>()) << "value is not table?";
    CHECK_EQ(v.size(), 1U);
    CHECK(v.find("mode") != nullptr);

    if (!moveParamSet_.loadValue(v, "move"))
        return false;
    if (!rensaParamSet_.loadValue(v, "main"))
        return false;

    return true;
}

toml::Value EvaluationParameterMap::toTomlValue() const
{
    toml::Value v1 = moveParamSet_.toTomlValue();
    toml::Value v2 = rensaParamSet_.toTomlValue();
    CHECK(v1.merge(v2));

    return v1;
}

string EvaluationParameterMap::toString() const
{
    stringstream ss;
    ss << toTomlValue();
    return ss.str();
}

void EvaluationParameterMap::removeNontokopuyoParameter()
{
    moveParamSet_.removeNontokopuyoParameter();
    rensaParamSet_.removeNontokopuyoParameter();
}
