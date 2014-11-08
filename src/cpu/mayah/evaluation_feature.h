#ifndef EVALUATION_FEATURE_H_
#define EVALUATION_FEATURE_H_

#include <string>
#include <vector>

enum EvaluationFeatureKey {
#define DEFINE_PARAM(NAME, tweakability) NAME,
#define DEFINE_SPARSE_PARAM(NAME, numValue, tweakability) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
};

enum EvaluationSparseFeatureKey {
#define DEFINE_PARAM(NAME, tweakability) /* ignored */
#define DEFINE_SPARSE_PARAM(NAME, numValue, tweakability) NAME,
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
};

enum class Tweakability {
    NOT_TWEAKABLE, IGNORE, TWEAKABLE
};

enum class SparseTweakability {
    NOT_TWEAKABLE, IGNORE, ASCENDING, DESCENDING
};

class EvaluationFeature {
public:
    EvaluationFeature(EvaluationFeatureKey key, Tweakability tweakability, const char* str)
        : key_(key), tweakability_(tweakability), str_(str) {}

    static const std::vector<EvaluationFeature>& all();
    static const EvaluationFeature& toFeature(EvaluationFeatureKey key) { return all()[key]; }

    EvaluationFeatureKey key() const { return key_; }
    Tweakability tweakability() const { return tweakability_; }
    const std::string& str() const { return str_; }

    bool tweakable() const { return tweakability_ == Tweakability::TWEAKABLE; }
    bool shouldIgnore() const { return tweakability_ == Tweakability::IGNORE; }

private:
    EvaluationFeatureKey key_;
    Tweakability tweakability_;
    std::string str_;
};

class EvaluationSparseFeature {
public:
    EvaluationSparseFeature(EvaluationSparseFeatureKey key, size_t size, SparseTweakability tweakability, const char* str) :
        key_(key), size_(size), tweakability_(tweakability), str_(str) {}

    static const std::vector<EvaluationSparseFeature>& all();
    static const EvaluationSparseFeature& toFeature(EvaluationSparseFeatureKey key) { return all()[key]; }

    EvaluationSparseFeatureKey key() const { return key_; }
    size_t size() const { return size_; }
    SparseTweakability tweakability() const { return tweakability_; }
    const std::string& str() const { return str_; }

    bool tweakable() const { return tweakability_ == SparseTweakability::ASCENDING || tweakability_ == SparseTweakability::DESCENDING; }
    bool shouldIgnore() const { return tweakability_ == SparseTweakability::IGNORE; }
    bool ascending() const { return tweakability_ == SparseTweakability::ASCENDING; }
    bool descending() const { return tweakability_ == SparseTweakability::DESCENDING; }

private:
    EvaluationSparseFeatureKey key_;
    int size_;
    SparseTweakability tweakability_;
    std::string str_;
};

#endif
