#include "evaluation_feature.h"

#include <initializer_list>

#include "core/field_constant.h"

using namespace std;

const vector<EvaluationFeature> g_allEvaluationFeatures {
#define DEFINE_PARAM(NAME, tweakable) EvaluationFeature(NAME, Tweakable::tweakable, #NAME),
#define DEFINE_SPARSE_PARAM(NAME, numValue, tweakable) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
};

const vector<EvaluationSparseFeature> g_allEvaluationSparseFeatures {
#define DEFINE_PARAM(NAME, tweakable) /* ignored */
#define DEFINE_SPARSE_PARAM(NAME, numValue, tweakable) EvaluationSparseFeature(NAME, numValue, Tweakable::tweakable, #NAME),
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
};

// static
const vector<EvaluationFeature>& EvaluationFeature::all()
{
    return g_allEvaluationFeatures;
}

// static
const vector<EvaluationSparseFeature>& EvaluationSparseFeature::all()
{
    return g_allEvaluationSparseFeatures;
}
