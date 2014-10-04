#include "feature_parameter.h"

#include <fstream>
#include <map>
#include <sstream>
#include <vector>

#include <gflags/gflags.h>

#include "core/field/core_field.h"

using namespace std;

FeatureParameter::FeatureParameter() :
    coef_(SIZE_OF_EVALUATION_FEATURE_KEY),
    sparseCoef_(SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY)
{
#define DEFINE_PARAM(key)
#define DEFINE_SPARSE_PARAM(key, maxValue) sparseCoef_[key].resize(maxValue);
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
}

FeatureParameter::FeatureParameter(const string& filename) :
    coef_(SIZE_OF_EVALUATION_FEATURE_KEY),
    sparseCoef_(SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY)
{
    CHECK(load(filename));
}

bool FeatureParameter::save(const string& filename)
{
    try {
        ofstream ofs(filename, ios::out | ios::trunc);

#define DEFINE_PARAM(key) ofs << (#key) << " = " << getValue(key) << endl;
#define DEFINE_SPARSE_PARAM(key, maxValue) ofs << (#key) << " =";      \
        for (int i = 0; i < (maxValue); ++i) {                         \
            ofs << " " << getValue(key, i);                            \
        }                                                              \
        ofs << endl;
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM

        return true;
    } catch (std::exception& e) {
        LOG(WARNING) << "FeatureParameter::save failed: " << e.what();
        return false;
    }
}

bool FeatureParameter::load(const string& filename)
{
#define DEFINE_PARAM(key) /* ignored */
#define DEFINE_SPARSE_PARAM(key, numValue) sparseCoef_[key].resize(numValue);
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM

    map<string, vector<double>> keyValues;

    try {
        ifstream ifs(filename, ios::in);

        string str;
        while (getline(ifs, str)) {
            istringstream ss(str);
            string key;
            if (!(ss >> key))
                continue;

            char c;
            if (!((ss >> c) && c == '=')) {
                LOG(WARNING) << "Invalid FeatureParameterFormat: "
                             << key << " : "
                             << str;
                return false;
            }

            vector<double> values;
            double v;
            while (ss >> v)
                values.push_back(v);

            keyValues.insert(make_pair(key, values));
        }

#define DEFINE_PARAM(key)                                               \
        if (keyValues.count(#key)) {                                    \
            coef_[key] = keyValues[#key].front();                       \
            keyValues.erase(#key);                                      \
        }
#define DEFINE_SPARSE_PARAM(key, maxValue)                              \
        if (keyValues.count(#key)) {                                    \
            if (maxValue != keyValues[#key].size()) {                   \
                LOG(ERROR) << "Invalid FeatureParameterFormat: "        \
                           << #key << " : Length mismatch.";            \
                return false;                                           \
            }                                                           \
            sparseCoef_[key] = keyValues[#key];                         \
            keyValues.erase(#key);                                      \
        }
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM

        if (!keyValues.empty()) {
            stringstream ss;
            bool first = true;
            for (const auto& entry : keyValues) {
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
    } catch (std::exception& e) {
        LOG(WARNING) << "FeatureParameter::load failed: " << e.what();
        return false;
    }
}

string FeatureParameter::toString() const
{
    ostringstream ss;

    // TODO(mayah): We should not use this kind of hack. Use for-loop.
#define DEFINE_PARAM(key) ss << (#key) << " = " << coef_[key] << endl;
#define DEFINE_SPARSE_PARAM(key, maxValue) ss << (#key) << " = ";       \
    for (int i = 0; i < (maxValue); ++i) {                              \
        ss << sparseCoef_[key][i] << ' ';                               \
    }                                                                   \
    ss << endl;
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM

    return ss.str();
}

bool operator==(const FeatureParameter& lhs, const FeatureParameter& rhs)
{
    return lhs.coef_ == rhs.coef_ && lhs.sparseCoef_ == rhs.sparseCoef_;
}
