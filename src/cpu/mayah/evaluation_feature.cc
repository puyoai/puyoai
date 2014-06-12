#include "evaluation_feature.h"

#include <fstream>
#include <map>
#include <sstream>
#include <vector>

using namespace std;

double PlanEvaluationFeature::calculateScore(const EvaluationParams& params) const
{
    double result = 0;
#define DEFINE_PARAM(name) result += get(name) * params.get(name);
#define DEFINE_RANGE_PARAM(name, maxValue) result += params.get(name, get(name));
#define DEFINE_SPARSE_PARAM(name, numValue) /* ignored */
#include "plan_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
#undef DEFINE_SPARSE_PARAM

    for (std::pair<PlanSparseFeatureParam, int> p : m_sparseFeatures)
        result += params.get(p.first, p.second);

    return result;
}

string PlanEvaluationFeature::toString() const
{
    // TODO(mayah): Not implemented yet.

    ostringstream ss;
    for (double d : m_features)
        ss << d << ' ';

    return ss.str();
}

double RensaEvaluationFeature::calculateScore(const EvaluationParams& params) const
{
    double result = 0;
#define DEFINE_PARAM(name) result += get(name) * params.get(name);
#define DEFINE_RANGE_PARAM(name, maxValue) result += params.get(name, get(name));
#include "rensa_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM

    return result;
}

const RensaEvaluationFeature EvaluationFeature::s_emptyRensaFeature;

string EvaluationFeature::toString() const
{
    // return "NOT_IMPLEMENTED_YET";

    return m_planFeature.toString();
}

const RensaEvaluationFeature& EvaluationFeature::findBestRensaFeature(const EvaluationParams& params) const
{
    if (m_rensaFeatures.empty())
        return s_emptyRensaFeature;

    auto result = m_rensaFeatures.begin();
    double resultScore = result->calculateScore(params);
    for (auto it = m_rensaFeatures.begin(); it != m_rensaFeatures.end(); ++it) {
        double score = it->calculateScore(params);
        if (resultScore < score) {
            result = it;
            resultScore = score;
        }
    }

    return *result;
}

double EvaluationFeature::calculateScoreWith(const EvaluationParams& params, const RensaEvaluationFeature& rensaFeature) const
{
    double planScore = m_planFeature.calculateScore(params);
    double rensaScore = rensaFeature.calculateScore(params);

    return planScore + rensaScore;
}

EvaluationParams::EvaluationParams(const char* filename) :
    m_planFeaturesCoef(SIZE_OF_PLAN_FEATURE_PARAM),
    m_planRangeFeaturesCoef(SIZE_OF_PLAN_RANGE_FEATURE_PARAM),
    m_planSparseFeaturesCoef(SIZE_OF_PLAN_SPARSE_FEATURE_PARAM),
    m_rensaFeaturesCoef(SIZE_OF_RENSA_FEATURE_PARAM),
    m_rensaRangeFeaturesCoef(SIZE_OF_RENSA_RANGE_FEATURE_PARAM)
{
#define DEFINE_PARAM(name) /* ignored */
#define DEFINE_RANGE_PARAM(name, maxValue) m_planRangeFeaturesCoef[name].resize(maxValue);
#define DEFINE_SPARSE_PARAM(name, numValue) m_planSparseFeaturesCoef[name].resize(numValue);
#include "plan_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
#undef DEFINE_SPARSE_PARAM

#define DEFINE_PARAM(name) /* ignored */
#define DEFINE_RANGE_PARAM(name, maxValue) m_rensaRangeFeaturesCoef[name].resize(maxValue);
#include "rensa_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM

    if (filename != nullptr)
        CHECK(load(filename));
}

bool EvaluationParams::save(const char* filename)
{
    try {
        ofstream ofs(filename, ios::out | ios::trunc);

#define DEFINE_PARAM(name) ofs << (#name) << " = " << get(name) << endl;
#define DEFINE_RANGE_PARAM(name, maxValue) ofs << (#name) << " =";     \
            for (int i = 0; i < (maxValue); ++i) {                     \
                ofs << " " << get(name, i);                            \
            }                                                          \
        ofs << endl;
#define DEFINE_SPARSE_PARAM(name, numValue) DEFINE_RANGE_PARAM(name, numValue)
#include "plan_evaluation_feature.tab"
#include "rensa_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
#undef DEFINE_SPARSE_PARAM

        return true;
    } catch (std::exception& e) {
        LOG(WARNING) << "EvaluationParams::save failed: " << e.what();
        return false;
    }
}

bool EvaluationParams::load(const char* filename)
{
    map<string, vector<double>> keyValues;

    try {
        ifstream ifs(filename, ios::in);

        string str;
        while (getline(ifs, str)) {
            istringstream ss(str);
            string name;
            if (!(ss >> name))
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

            keyValues.insert(make_pair(name, values));
        }

#define DEFINE_PARAM(name) if (keyValues.count(#name)) {                \
            set(name, keyValues[#name].front());                        \
        }
#define DEFINE_RANGE_PARAM(name, maxValue) if (keyValues.count(#name)) { \
            if (maxValue != keyValues[#name].size()) {                  \
                LOG(WARNING) << "Invalid EvaluationFeatureFormat: Length mismatch."; \
                return false;                                           \
            }                                                           \
            set(name, keyValues[#name]);                                \
        }
#define DEFINE_SPARSE_PARAM(name, numValue) DEFINE_RANGE_PARAM(name, numValue)
#include "plan_evaluation_feature.tab"
#include "rensa_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM

        return true;
    } catch (std::exception& e) {
        LOG(WARNING) << "EvaluationParams::load failed: " << e.what();
        return false;
    }
}

string EvaluationParams::toString() const
{
    ostringstream ss;

    // TODO(mayah): We should not use this kind of hack. Use for-loop.
#define DEFINE_PARAM(name) ss << (#name) << " = " << get(name) << endl;
#define DEFINE_RANGE_PARAM(name, maxValue) ss << (#name) << " = "; \
    for (int i = 0; i < (maxValue); ++i) {                              \
        ss << get(name, i) << ' ';                   \
    }                                                                   \
    ss << endl;
#define DEFINE_SPARSE_PARAM(name, numValue) DEFINE_RANGE_PARAM(name, numValue)
#include "plan_evaluation_feature.tab"
#include "rensa_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM

    return ss.str();
}

bool operator==(const EvaluationParams& lhs, const EvaluationParams& rhs)
{
    return lhs.m_planFeaturesCoef == rhs.m_planFeaturesCoef &&
        lhs.m_planRangeFeaturesCoef == rhs.m_planRangeFeaturesCoef &&
        lhs.m_rensaFeaturesCoef == rhs.m_rensaFeaturesCoef &&
        lhs.m_rensaRangeFeaturesCoef == rhs.m_rensaRangeFeaturesCoef;
}
