#ifndef EVALUATION_FEATURE_H_
#define EVALUATION_FEATURE_H_

#include <cstddef>
#include <string>
#include <vector>

#include "core/field_constant.h"

enum class Tweakability {
    NOT_TWEAKABLE, IGNORE, TWEAKABLE
};

enum class SparseTweakability {
    NOT_TWEAKABLE, IGNORE, ASCENDING, DESCENDING
};

enum EvaluationMoveFeatureKey {
#define DEFINE_MOVE_PARAM(NAME, tweakability) NAME,
#define DEFINE_MOVE_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#define DEFINE_RENSA_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_RENSA_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_MOVE_PARAM
#undef DEFINE_MOVE_SPARSE_PARAM
#undef DEFINE_RENSA_PARAM
#undef DEFINE_RENSA_SPARSE_PARAM
};

enum EvaluationRensaFeatureKey {
#define DEFINE_MOVE_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_MOVE_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#define DEFINE_RENSA_PARAM(NAME, tweakability) NAME,
#define DEFINE_RENSA_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_MOVE_PARAM
#undef DEFINE_MOVE_SPARSE_PARAM
#undef DEFINE_RENSA_PARAM
#undef DEFINE_RENSA_SPARSE_PARAM
};

enum EvaluationMoveSparseFeatureKey {
#define DEFINE_MOVE_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_MOVE_SPARSE_PARAM(NAME, numValue, tweakability) NAME,
#define DEFINE_RENSA_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_RENSA_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_MOVE_PARAM
#undef DEFINE_MOVE_SPARSE_PARAM
#undef DEFINE_RENSA_PARAM
#undef DEFINE_RENSA_SPARSE_PARAM
};

enum EvaluationRensaSparseFeatureKey {
#define DEFINE_MOVE_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_MOVE_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#define DEFINE_RENSA_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_RENSA_SPARSE_PARAM(NAME, numValue, tweakability) NAME,
#include "evaluation_feature.tab"
#undef DEFINE_MOVE_PARAM
#undef DEFINE_MOVE_SPARSE_PARAM
#undef DEFINE_RENSA_PARAM
#undef DEFINE_RENSA_SPARSE_PARAM
};

template<typename FeatureKey>
class EvaluationFeature {
public:
    EvaluationFeature(FeatureKey key, Tweakability tweakability, const char* name)
        : key_(key), tweakability_(tweakability), name_(name) {}

    FeatureKey key() const { return key_; }
    Tweakability tweakability() const { return tweakability_; }
    const std::string& name() const { return name_; }

    bool isTweakable() const { return tweakability_ == Tweakability::TWEAKABLE; }
    bool shouldIgnore() const { return tweakability_ == Tweakability::IGNORE; }

private:
    FeatureKey key_;
    Tweakability tweakability_;
    std::string name_;
};

typedef EvaluationFeature<EvaluationMoveFeatureKey> EvaluationMoveFeature;
typedef EvaluationFeature<EvaluationRensaFeatureKey> EvaluationRensaFeature;

template<typename FeatureKey>
class EvaluationSparseFeature {
public:
    EvaluationSparseFeature(FeatureKey key, size_t size, SparseTweakability tweakability, const char* name) :
        key_(key), size_(size), tweakability_(tweakability), name_(name) {}

    FeatureKey key() const { return key_; }
    size_t size() const { return size_; }
    SparseTweakability tweakability() const { return tweakability_; }
    const std::string& name() const { return name_; }

    bool isTweakable() const { return tweakability_ == SparseTweakability::ASCENDING || tweakability_ == SparseTweakability::DESCENDING; }
    bool shouldIgnore() const { return tweakability_ == SparseTweakability::IGNORE; }
    bool isAscending() const { return tweakability_ == SparseTweakability::ASCENDING; }
    bool isDescending() const { return tweakability_ == SparseTweakability::DESCENDING; }

private:
    FeatureKey key_;
    int size_;
    SparseTweakability tweakability_;
    std::string name_;
};

typedef EvaluationSparseFeature<EvaluationMoveSparseFeatureKey> EvaluationMoveSparseFeature;
typedef EvaluationSparseFeature<EvaluationRensaSparseFeatureKey> EvaluationRensaSparseFeature;

// ----------------------------------------------------------------------

class EvaluationMoveFeatures {
public:
    static const EvaluationMoveFeature& feature(EvaluationMoveFeatureKey key) { return features_[key]; }
    int size() const { return end() - begin(); }
    const EvaluationMoveFeature* begin() const;
    const EvaluationMoveFeature* end() const;
private:
    static const EvaluationMoveFeature features_[];
};

class EvaluationMoveSparseFeatures {
public:
    static const EvaluationMoveSparseFeature& feature(EvaluationMoveSparseFeatureKey key) { return features_[key]; }
    int size() const { return end() - begin(); }
    const EvaluationMoveSparseFeature* begin() const;
    const EvaluationMoveSparseFeature* end() const;
private:
    static const EvaluationMoveSparseFeature features_[];
};

class EvaluationRensaFeatures {
public:
    static const EvaluationRensaFeature& feature(EvaluationRensaFeatureKey key) { return features_[key]; }
    int size() const { return end() - begin(); }
    const EvaluationRensaFeature* begin() const;
    const EvaluationRensaFeature* end() const;
private:
    static const EvaluationRensaFeature features_[];
};

class EvaluationRensaSparseFeatures {
public:
    static const EvaluationRensaSparseFeature& feature(EvaluationRensaSparseFeatureKey key) { return features_[key]; }
    int size() const { return end() - begin(); }
    const EvaluationRensaSparseFeature* begin() const;
    const EvaluationRensaSparseFeature* end() const;
private:
    static const EvaluationRensaSparseFeature features_[];
};

inline const EvaluationMoveFeature& toFeature(EvaluationMoveFeatureKey key)
{
    return EvaluationMoveFeatures::feature(key);
}
inline const EvaluationMoveSparseFeature& toFeature(EvaluationMoveSparseFeatureKey key)
{
    return EvaluationMoveSparseFeatures::feature(key);
}
inline const EvaluationRensaFeature& toFeature(EvaluationRensaFeatureKey key)
{
    return EvaluationRensaFeatures::feature(key);
}
inline const EvaluationRensaSparseFeature& toFeature(EvaluationRensaSparseFeatureKey key)
{
    return EvaluationRensaSparseFeatures::feature(key);
}

// ----------------------------------------------------------------------

class EvaluationMoveFeatureSet {
public:
  typedef EvaluationMoveFeatureKey FeatureKey;
  typedef EvaluationMoveSparseFeatureKey SparseFeatureKey;

  static EvaluationMoveFeatures features() { return EvaluationMoveFeatures(); }
  static EvaluationMoveSparseFeatures sparseFeatures() { return EvaluationMoveSparseFeatures(); }
};

class EvaluationRensaFeatureSet {
public:
  typedef EvaluationRensaFeatureKey FeatureKey;
  typedef EvaluationRensaSparseFeatureKey SparseFeatureKey;

  static EvaluationRensaFeatures features() { return EvaluationRensaFeatures(); }
  static EvaluationRensaSparseFeatures sparseFeatures() { return EvaluationRensaSparseFeatures(); }
};

#endif // EVALUATION_FEATURE_H_
