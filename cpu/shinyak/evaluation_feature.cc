#include "evaluation_feature.h"

#include <glog/logging.h>
#include <sstream>
#include <stdio.h>
#include "field.h"

using namespace std;

double PlanEvaluationFeature::calculateScore(const EvaluationParams& params) const
{
    double result = 0;
#define DEFINE_PARAM(name) result += get(name) * params.get(name);
#define DEFINE_RANGE_PARAM(name, maxValue) result += params.get(name, get(name));
#include "plan_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM

    return result;
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
    // TODO(mayah): We have to implement this.
    return "NOT_IMPLEMENTED_YET";
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

EvaluationParams::EvaluationParams() :
    m_planFeaturesCoef(SIZE_OF_PLAN_FEATURE_PARAM),
    m_planRangeFeaturesCoef(SIZE_OF_PLAN_RANGE_FEATURE_PARAM),
    m_rensaFeaturesCoef(SIZE_OF_RENSA_FEATURE_PARAM),
    m_rensaRangeFeaturesCoef(SIZE_OF_RENSA_RANGE_FEATURE_PARAM)
{
#define DEFINE_PARAM(name) /* ignored */
#define DEFINE_RANGE_PARAM(name, maxValue) m_planRangeFeaturesCoef[name].resize(maxValue);
#include "plan_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM

#define DEFINE_PARAM(name) /* ignored */
#define DEFINE_RANGE_PARAM(name, maxValue) m_rensaRangeFeaturesCoef[name].resize(maxValue);
#include "rensa_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM

    initialize();
}

void EvaluationParams::initialize()
{
    // TODO(mayah): Sort this.

    set(TOTAL_FRAMES, 0.0);
    set(TOTAL_FRAMES_INVERSE, 1.0);

    set(MAX_RENSA_NECESSARY_PUYOS, 0);
    set(MAX_RENSA_NECESSARY_PUYOS_INVERSE, 1.0);

    set(CONNECTION_1, -0.1 / 30.0);
    set(CONNECTION_2,  0.9 / 30.0);
    set(CONNECTION_3,  1.2 / 30.0);
    set(CONNECTION_4, -0.5 / 30.0);

    set(DENSITY_0,  0.02);
    set(DENSITY_1, -0.024);
    set(DENSITY_2,  0.003);
    set(DENSITY_3,  0.003);
    set(DENSITY_4,  0.004);

    set(CONNECTION_AFTER_VANISH_1, -0.1 / 15.0);
    set(CONNECTION_AFTER_VANISH_2,  0.9 / 15.0);
    set(CONNECTION_AFTER_VANISH_3,  1.6 / 15.0);
    set(CONNECTION_AFTER_VANISH_4,  0.5 / 15.0);

    set(HAND_WIDTH_1, 0.1);
    set(HAND_WIDTH_2, 0.1);
    set(HAND_WIDTH_3, 0.1);
    set(HAND_WIDTH_4, 0.1);

    set(HAND_WIDTH_RATIO_21, 0.1);
    set(HAND_WIDTH_RATIO_32, 0.1);
    set(HAND_WIDTH_RATIO_43, 0.1);
    set(HAND_WIDTH_RATIO_21_SQUARED, 0.1);
    set(HAND_WIDTH_RATIO_32_SQUARED, 0.1);
    set(HAND_WIDTH_RATIO_43_SQUARED, 0.1);

    set(SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, 0.0);
    set(SQUARE_SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, -0.1);

    set(EMPTY_AVAILABILITY_00, 0.95);
    set(EMPTY_AVAILABILITY_01, 0.90);
    set(EMPTY_AVAILABILITY_02, 0.85);
    set(EMPTY_AVAILABILITY_11, 0.80);
    set(EMPTY_AVAILABILITY_12, 0.75);
    set(EMPTY_AVAILABILITY_22, 0.30);

    set(STRATEGY_LARGE_ENOUGH, 100.0);
    set(STRATEGY_ZENKESHI, 80.0);
    set(STRATEGY_TAIOU, 90.0);
    set(STRATEGY_TSUBUSHI, 70.0);
    set(STRATEGY_HOUWA, -60.0);
    set(STRATEGY_SAKIUCHI, -20.0);
    set(STRATEGY_SCORE, 0.001);

    for (int i = 0; i < 20; ++i)
        set(MAX_CHAINS, i, i);
    set(THIRD_COLUMN_HEIGHT,  9, -0.5);
    set(THIRD_COLUMN_HEIGHT, 10, -2.0);
    set(THIRD_COLUMN_HEIGHT, 11, -2.0);
    set(THIRD_COLUMN_HEIGHT, 12, -2.0);
    set(THIRD_COLUMN_HEIGHT, 13, -2.0);
    set(THIRD_COLUMN_HEIGHT, 14, -2.0);
    set(THIRD_COLUMN_HEIGHT, 15, -2.0);
}

bool EvaluationParams::save(const char* filename)
{
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("save");
        return false;
    }

    // TODO(mayah): We assume vector memory is continuously allocated.
    fwrite(&m_planFeaturesCoef[0], sizeof(double), m_planFeaturesCoef.size(), fp);
    for (int i = 0; i < SIZE_OF_PLAN_RANGE_FEATURE_PARAM; ++i)
        fwrite(&m_planRangeFeaturesCoef[i][0], sizeof(double), m_planRangeFeaturesCoef.size(), fp);

    fwrite(&m_rensaFeaturesCoef[0], sizeof(double), m_rensaRangeFeaturesCoef.size(), fp);
    for (int i = 0; i < SIZE_OF_RENSA_RANGE_FEATURE_PARAM; ++i)
        fwrite(&m_rensaRangeFeaturesCoef[i][0], sizeof(double), m_rensaRangeFeaturesCoef.size(), fp);

    fclose(fp);
    return true;
}

bool EvaluationParams::load(const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("load");
        return false;
    }

    // TODO(mayah): We assume vector memory is continuously allocated.
    fread(&m_planFeaturesCoef[0], sizeof(double), m_planFeaturesCoef.size(), fp);
    for (int i = 0; i < SIZE_OF_PLAN_RANGE_FEATURE_PARAM; ++i)
        fread(&m_planRangeFeaturesCoef[i][0], sizeof(double), m_planRangeFeaturesCoef[i].size(), fp);

    fread(&m_rensaFeaturesCoef[0], sizeof(double), m_rensaFeaturesCoef.size(), fp);
    for (int i = 0; i < SIZE_OF_RENSA_RANGE_FEATURE_PARAM; ++i)
        fread(&m_rensaRangeFeaturesCoef[i][0], sizeof(double), m_rensaRangeFeaturesCoef[i].size(), fp);

    fclose(fp);
    return true;
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
#include "plan_evaluation_feature.tab"
#include "rensa_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM

    return ss.str();
}
