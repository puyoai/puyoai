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
    if (!mainRensaParamSet_.loadValue(v, "main"))
        return false;
    if (!sideRensaParamSet_.loadValue(v, "side"))
        return false;

    return true;
}

toml::Value EvaluationParameterMap::toTomlValue() const
{
    toml::Value v = toml::Table();

    toml::Value v1 = moveParamSet_.toTomlValue("move");
    if (v1.valid())
        CHECK(v.merge(v1));

    toml::Value v2 = mainRensaParamSet_.toTomlValue("main");
    if (v2.valid())
        CHECK(v.merge(v2));

    toml::Value v3 = sideRensaParamSet_.toTomlValue("side");
    if (v3.valid())
        CHECK(v.merge(v3));

    return v;
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
    mainRensaParamSet_.removeNontokopuyoParameter();
    // For tokopuyo, we don't think about side-rensa.
    sideRensaParamSet_.clear();
}
