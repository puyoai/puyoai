#ifndef __EVALUATION_FEATURE_H_
#define __EVALUATION_FEATURE_H_

#include <string>
#include <vector>
#include <glog/logging.h>

class EvaluationParams;

#define USE_CONNECTION_FEATURE 0
#define USE_EMPTY_AVAILABILITY_FEATURE 0
#define USE_HAND_WIDTH_FEATURE 0
#define USE_THIRD_COLUMN_HEIGHT_FEATURE 0

enum PlanFeatureParam {
#define DEFINE_PARAM(NAME) NAME,
#define DEFINE_RANGE_PARAM(NAME, maxValue) /* ignored */
#define DEFINE_SPARSE_PARAM(NAME, numValue) /* ignore */
#include "plan_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
#undef DEFINE_SPARSE_PARAM
    SIZE_OF_PLAN_FEATURE_PARAM
};

inline PlanFeatureParam toPlanFeatureParam(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_PLAN_FEATURE_PARAM);
    return static_cast<PlanFeatureParam>(ith);
}
    
enum PlanRangeFeatureParam {
#define DEFINE_PARAM(NAME) /* ignored */
#define DEFINE_RANGE_PARAM(NAME, maxValue) NAME,
#define DEFINE_SPARSE_PARAM(NAME, numValue) /* ignored */
#include "plan_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
#undef DEFINE_SPARSE_PARAM
    SIZE_OF_PLAN_RANGE_FEATURE_PARAM
};

inline PlanRangeFeatureParam toPlanRangeFeatureParam(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_PLAN_RANGE_FEATURE_PARAM);
    return static_cast<PlanRangeFeatureParam>(ith);
}

enum PlanSparseFeatureParam {
#define DEFINE_PARAM(NAME) /* ignored */
#define DEFINE_RANGE_PARAM(NAME, maxValue) /* ignored */
#define DEFINE_SPARSE_PARAM(NAME, numValue) NAME,
#include "plan_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
#undef DEFINE_SPARSE_PARAM
    SIZE_OF_PLAN_SPARSE_FEATURE_PARAM    
};

inline PlanSparseFeatureParam toPlanSparseFeatureParam(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_PLAN_SPARSE_FEATURE_PARAM);
    return static_cast<PlanSparseFeatureParam>(ith);
}

enum RensaFeatureParam {
#define DEFINE_PARAM(NAME) NAME,
#define DEFINE_RANGE_PARAM(NAME, maxValue) /* ignored */
#include "rensa_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
    SIZE_OF_RENSA_FEATURE_PARAM
};

inline RensaFeatureParam toRensaFeatureParam(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_RENSA_FEATURE_PARAM);
    return static_cast<RensaFeatureParam>(ith);
}

enum RensaRangeFeatureParam {
#define DEFINE_PARAM(NAME) /* ignored */
#define DEFINE_RANGE_PARAM(NAME, maxValue) NAME,
#include "rensa_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
    SIZE_OF_RENSA_RANGE_FEATURE_PARAM
};

inline RensaRangeFeatureParam toRensaRangeFeatureParam(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_RENSA_RANGE_FEATURE_PARAM);
    return static_cast<RensaRangeFeatureParam>(ith);
}

class RensaEvaluationFeature {
public:
    RensaEvaluationFeature() :
        m_features(SIZE_OF_RENSA_FEATURE_PARAM),
        m_rangeFeatures(SIZE_OF_RENSA_RANGE_FEATURE_PARAM)
    {}

    double get(RensaFeatureParam param) const { return m_features[param]; }
    void set(RensaFeatureParam param, double value) { m_features[param] = value; }
    void add(RensaFeatureParam param, double value) { m_features[param] += value; }

    int get(RensaRangeFeatureParam param) const { return m_rangeFeatures[param]; }
    void set(RensaRangeFeatureParam param, int value) { m_rangeFeatures[param] = value; }

public:
    double calculateScore(const EvaluationParams&) const;

private:
    std::vector<double> m_features;
    std::vector<int> m_rangeFeatures;
};

class PlanEvaluationFeature {
public:
    PlanEvaluationFeature() :
        m_features(SIZE_OF_PLAN_FEATURE_PARAM),
        m_rangeFeatures(SIZE_OF_PLAN_RANGE_FEATURE_PARAM)
    {
    }

public:
    double get(PlanFeatureParam param) const { return m_features[param]; }
    void set(PlanFeatureParam param, double value) { m_features[param] = value; }
    void add(PlanFeatureParam param, double value) { m_features[param] += value; }

    void set(PlanRangeFeatureParam param, int value) { m_rangeFeatures[param] = value; }
    int get(PlanRangeFeatureParam param) const { return m_rangeFeatures[param]; }

    void add(PlanSparseFeatureParam param, int value) { m_sparseFeatures.push_back(std::make_pair(param, value)); }
    const std::vector<std::pair<PlanSparseFeatureParam, int>>& sparseFeatures() const { return m_sparseFeatures; }    
public:
    std::string toString() const;

public:
    double calculateScore(const EvaluationParams&) const;

private:
    // TODO(mayah): We would like to use std::array instead.
    std::vector<double> m_features;
    std::vector<int> m_rangeFeatures;
    std::vector<std::pair<PlanSparseFeatureParam, int>> m_sparseFeatures;
};

// TODO: Maybe we have to have bestEvaluationFeature in this class.
class EvaluationFeature {
public:
    const PlanEvaluationFeature& planFeature() const { return m_planFeature; }
    PlanEvaluationFeature& modifiablePlanFeature() { return m_planFeature; }

    const RensaEvaluationFeature& findBestRensaFeature(const EvaluationParams&) const;
    size_t numRensaFeatures() const { return m_rensaFeatures.size(); }    
    void addRensaFeature(const RensaEvaluationFeature& rensaEvaluationFeature) { m_rensaFeatures.push_back(rensaEvaluationFeature); }

public:
    double calculateScore(const EvaluationParams& params) const { return calculateScoreWith(params, findBestRensaFeature(params)); }
    // Calculates score with the specified RensaEvaluationFeature.
    double calculateScoreWith(const EvaluationParams&, const RensaEvaluationFeature&) const;

    std::string toString() const;

private:
    PlanEvaluationFeature m_planFeature;
    std::vector<RensaEvaluationFeature> m_rensaFeatures;

    static const RensaEvaluationFeature s_emptyRensaFeature;
};

class EvaluationParams {
public:
    // If filename is NULL, all the parameters will be 0.
    explicit EvaluationParams(const char* filename);

    double get(PlanFeatureParam param) const { return m_planFeaturesCoef[param]; }
    void set(PlanFeatureParam param, double value) { m_planFeaturesCoef[param] = value; }
    void add(PlanFeatureParam param, double value) { m_planFeaturesCoef[param] += value; }

    double get(PlanRangeFeatureParam param, int index) const { return m_planRangeFeaturesCoef[param][index]; }
    void set(PlanRangeFeatureParam param, int i, double value) { m_planRangeFeaturesCoef[param][i] = value; }
    void set(PlanRangeFeatureParam param, const std::vector<double>& values) { m_planRangeFeaturesCoef[param] = values; }
    void add(PlanRangeFeatureParam param, int i, double value) { m_planRangeFeaturesCoef[param][i] += value; }

    double get(PlanSparseFeatureParam param, int index) const { return m_planSparseFeaturesCoef[param][index]; }
    void set(PlanSparseFeatureParam param, int i, double value) { m_planSparseFeaturesCoef[param][i] = value; }
    void set(PlanSparseFeatureParam param, const std::vector<double>& values) { m_planSparseFeaturesCoef[param] = values; }
    void add(PlanSparseFeatureParam param, int i, double value) { m_planSparseFeaturesCoef[param][i] += value; }

    double get(RensaFeatureParam param) const { return m_rensaFeaturesCoef[param]; }
    void set(RensaFeatureParam param, double value) { m_rensaFeaturesCoef[param] = value; }
    void add(RensaFeatureParam param, double value) { m_rensaFeaturesCoef[param] += value; }

    double get(RensaRangeFeatureParam param, int index) const { return m_rensaRangeFeaturesCoef[param][index]; }
    void set(RensaRangeFeatureParam param, int i, double value) { m_rensaRangeFeaturesCoef[param][i] = value; }
    void set(RensaRangeFeatureParam param, const std::vector<double>& values) { m_rensaRangeFeaturesCoef[param] = values; }
    void add(RensaRangeFeatureParam param, int i, double value) { m_rensaRangeFeaturesCoef[param][i] += value; }

public:
    std::string toString() const;

    bool save(const char* filename);
    bool load(const char* filename);

    friend bool operator==(const EvaluationParams&, const EvaluationParams&);

private:
    std::vector<double> m_planFeaturesCoef;
    std::vector<std::vector<double> > m_planRangeFeaturesCoef;
    std::vector<std::vector<double> > m_planSparseFeaturesCoef;
    std::vector<double> m_rensaFeaturesCoef;
    std::vector<std::vector<double> > m_rensaRangeFeaturesCoef;
};

#endif
