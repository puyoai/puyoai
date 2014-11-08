#ifndef EVALUATION_FEATURE_KEY_H_
#define EVALUATION_FEATURE_KEY_H_

#include <glog/logging.h>

enum EvaluationFeatureKey {
#define DEFINE_PARAM(NAME, tweakable) NAME,
#define DEFINE_SPARSE_PARAM(NAME, numValue, tweakable) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
    SIZE_OF_EVALUATION_FEATURE_KEY
};

enum EvaluationSparseFeatureKey {
#define DEFINE_PARAM(NAME, tweakable) /* ignored */
#define DEFINE_SPARSE_PARAM(NAME, numValue, tweakable) NAME,
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
    SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY
};

inline EvaluationFeatureKey toEvaluationFeatureKey(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_EVALUATION_FEATURE_KEY);
    return static_cast<EvaluationFeatureKey>(ith);
}

inline std::string toString(EvaluationFeatureKey key)
{
    switch (key) {
#define DEFINE_PARAM(key, tweakable) case key: return #key;
#define DEFINE_SPARSE_PARAM(key, value, tweakable) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
    case SIZE_OF_EVALUATION_FEATURE_KEY:
        break;
    }

    CHECK(false) << "Unexpected EvaluationFeatureKey: " << static_cast<int>(key);
    return "";
}

inline EvaluationSparseFeatureKey toEvaluationSparseFeatureKey(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY);
    return static_cast<EvaluationSparseFeatureKey>(ith);
}

inline std::string toString(EvaluationSparseFeatureKey key)
{
    switch (key) {
#define DEFINE_PARAM(key, tweakable) /* ignored */
#define DEFINE_SPARSE_PARAM(key, value, tweakable) case key: return #key;
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
    case SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY:
        break;
    }

    CHECK(false) << "Unexpected EvaluationSparseFeatureKey: " << static_cast<int>(key);
    return "";
}

#endif
