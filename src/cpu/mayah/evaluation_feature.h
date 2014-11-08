#ifndef EVALUATION_FEATURE_H_
#define EVALUATION_FEATURE_H_

#include <string>
#include <vector>

enum EvaluationFeatureKey {
#define DEFINE_PARAM(NAME, tweakable) NAME,
#define DEFINE_SPARSE_PARAM(NAME, numValue, tweakable) /* ignored */
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
};

enum EvaluationSparseFeatureKey {
#define DEFINE_PARAM(NAME, tweakable) /* ignored */
#define DEFINE_SPARSE_PARAM(NAME, numValue, tweakable) NAME,
#include "evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_SPARSE_PARAM
};

enum class Tweakable {
    TWEAKABLE, NOT_TWEAKABLE, IGNORE, ASCENDING, DESCENDING
};

class EvaluationFeature {
public:
    EvaluationFeature(EvaluationFeatureKey key, Tweakable tweakable, const char* str)
        : key_(key), tweakable_(tweakable), str_(str) {}

    static const std::vector<EvaluationFeature>& all();
    static const EvaluationFeature& toFeature(EvaluationFeatureKey key) { return all()[key]; }

    EvaluationFeatureKey key() const { return key_; }
    const std::string& str() const { return str_; }

private:
    EvaluationFeatureKey key_;
    Tweakable tweakable_;
    std::string str_;
};

class EvaluationSparseFeature {
public:
    EvaluationSparseFeature(EvaluationSparseFeatureKey key, size_t size, Tweakable tweakable, const char* str) :
        key_(key), size_(size), tweakable_(tweakable), str_(str) {}

    static const std::vector<EvaluationSparseFeature>& all();
    static const EvaluationSparseFeature& toFeature(EvaluationSparseFeatureKey key) { return all()[key]; }

    EvaluationSparseFeatureKey key() const { return key_; }
    size_t size() const { return size_; }
    const std::string& str() const { return str_; }

private:
    EvaluationSparseFeatureKey key_;
    int size_;
    Tweakable tweakable_;
    std::string str_;
};

#endif
