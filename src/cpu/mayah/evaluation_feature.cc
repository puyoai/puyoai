#include "evaluation_feature.h"

const EvaluationMoveFeature EvaluationMoveFeatures::features_[] {
#define DEFINE_MOVE_PARAM(NAME, tweakability) EvaluationMoveFeature(NAME, Tweakability::tweakability, #NAME),
#define DEFINE_MOVE_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#define DEFINE_RENSA_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_RENSA_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_MOVE_PARAM
#undef DEFINE_MOVE_SPARSE_PARAM
#undef DEFINE_RENSA_PARAM
#undef DEFINE_RENSA_SPARSE_PARAM
};

const EvaluationMoveFeature* EvaluationMoveFeatures::begin() const { return std::begin(features_); }
const EvaluationMoveFeature* EvaluationMoveFeatures::end() const { return std::end(features_); }

const EvaluationMoveSparseFeature EvaluationMoveSparseFeatures::features_[] {
#define DEFINE_MOVE_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_MOVE_SPARSE_PARAM(NAME, numValue, tweakability) EvaluationMoveSparseFeature(NAME, numValue, SparseTweakability::tweakability, #NAME),
#define DEFINE_RENSA_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_RENSA_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_MOVE_PARAM
#undef DEFINE_MOVE_SPARSE_PARAM
#undef DEFINE_RENSA_PARAM
#undef DEFINE_RENSA_SPARSE_PARAM
};

const EvaluationMoveSparseFeature* EvaluationMoveSparseFeatures::begin() const { return std::begin(features_); }
const EvaluationMoveSparseFeature* EvaluationMoveSparseFeatures::end() const { return std::end(features_); }

const EvaluationRensaFeature EvaluationRensaFeatures::features_[] {
#define DEFINE_MOVE_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_MOVE_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#define DEFINE_RENSA_PARAM(NAME, tweakability) EvaluationRensaFeature(NAME, Tweakability::tweakability, #NAME),
#define DEFINE_RENSA_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_MOVE_PARAM
#undef DEFINE_MOVE_SPARSE_PARAM
#undef DEFINE_RENSA_PARAM
#undef DEFINE_RENSA_SPARSE_PARAM
};

const EvaluationRensaFeature* EvaluationRensaFeatures::begin() const { return std::begin(features_); }
const EvaluationRensaFeature* EvaluationRensaFeatures::end() const { return std::end(features_); }

const EvaluationRensaSparseFeature EvaluationRensaSparseFeatures::features_[] {
#define DEFINE_MOVE_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_MOVE_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#define DEFINE_RENSA_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_RENSA_SPARSE_PARAM(NAME, numValue, tweakability) EvaluationRensaSparseFeature(NAME, numValue, SparseTweakability::tweakability, #NAME),
#include "evaluation_feature.tab"
#undef DEFINE_MOVE_PARAM
#undef DEFINE_MOVE_SPARSE_PARAM
#undef DEFINE_RENSA_PARAM
#undef DEFINE_RENSA_SPARSE_PARAM
};

const EvaluationRensaSparseFeature* EvaluationRensaSparseFeatures::begin() const { return std::begin(features_); }
const EvaluationRensaSparseFeature* EvaluationRensaSparseFeatures::end() const { return std::end(features_); }
