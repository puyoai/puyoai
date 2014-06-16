#ifndef EVALUATION_FEATURE_KEY_H_
#define EVALUATION_FEATURE_KEY_H_

#include <glog/logging.h>

enum EvaluationFeatureKey {
#define DEFINE_PARAM(NAME) NAME,
#define DEFINE_SPARSE_PARAM(NAME, numValue) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
    SIZE_OF_EVALUATION_FEATURE_KEY
};

enum EvaluationSparseFeatureKey {
#define DEFINE_PARAM(NAME) /* ignored */
#define DEFINE_SPARSE_PARAM(NAME, numValue) NAME,
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

inline EvaluationSparseFeatureKey toEvaluationSparseFeatureKey(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY);
    return static_cast<EvaluationSparseFeatureKey>(ith);
}

#endif
