#include "evaluation_feature.h"

#include <fstream>
#include <map>
#include <sstream>
#include <vector>

#include "core/field/core_field.h"

using namespace std;

EvaluationFeature::EvaluationFeature(const char* filename) :
    featuresCoef_(SIZE_OF_EVALUATION_FEATURE_KEY),
    sparseFeaturesCoef_(SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY)
{
#define DEFINE_PARAM(key) /* ignored */
#define DEFINE_SPARSE_PARAM(key, numValue) sparseFeaturesCoef_[key].resize(numValue);
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM

    if (filename != nullptr)
        CHECK(load(filename));
}

bool EvaluationFeature::save(const char* filename)
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
        LOG(WARNING) << "EvaluationFeature::save failed: " << e.what();
        return false;
    }
}

bool EvaluationFeature::load(const char* filename)
{
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
                LOG(WARNING) << "Invalid EvaluationFeatureFormat: " << str;
                return false;
            }

            vector<double> values;
            double v;
            while (ss >> v)
                values.push_back(v);

            keyValues.insert(make_pair(key, values));
        }

#define DEFINE_PARAM(key) if (keyValues.count(#key)) {                \
            setValue(key, keyValues[#key].front());                   \
        }
#define DEFINE_SPARSE_PARAM(key, maxValue) if (keyValues.count(#key)) { \
            if (maxValue != keyValues[#key].size()) {                   \
                LOG(WARNING) << "Invalid EvaluationFeatureFormat: Length mismatch."; \
                return false;                                           \
            }                                                           \
            setValue(key, keyValues[#key]);                             \
        }
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM

        return true;
    } catch (std::exception& e) {
        LOG(WARNING) << "EvaluationFeature::load failed: " << e.what();
        return false;
    }
}

string EvaluationFeature::toString() const
{
    ostringstream ss;

    // TODO(mayah): We should not use this kind of hack. Use for-loop.
#define DEFINE_PARAM(key) ss << (#key) << " = " << getValue(key) << endl;
#define DEFINE_SPARSE_PARAM(key, maxValue) ss << (#key) << " = ";       \
    for (int i = 0; i < (maxValue); ++i) {                              \
        ss << getValue(key, i) << ' ';                                  \
    }                                                                   \
    ss << endl;
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM

    return ss.str();
}

bool operator==(const EvaluationFeature& lhs, const EvaluationFeature& rhs)
{
    return lhs.featuresCoef_ == rhs.featuresCoef_ &&
        lhs.sparseFeaturesCoef_ == rhs.sparseFeaturesCoef_;
}
