#include "evaluation_feature.h"

using namespace std;

const vector<EvaluationFeature> g_allEvaluationFeatures {
#define DEFINE_PARAM(NAME, tweakability) EvaluationFeature(NAME, Tweakability::tweakability, #NAME),
#define DEFINE_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
};

const vector<EvaluationSparseFeature> g_allEvaluationSparseFeatures {
#define DEFINE_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_SPARSE_PARAM(NAME, numValue, tweakability) EvaluationSparseFeature(NAME, numValue, SparseTweakability::tweakability, #NAME),
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
