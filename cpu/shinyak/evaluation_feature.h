#ifndef __EVALUATION_FEATURE_H_
#define __EVALUATION_FEATURE_H_

#include <string>
#include <vector>
#include <glog/logging.h>

class EvaluationParams;

enum PlanFeatureParam {
#define DEFINE_PARAM(NAME) NAME,
#define DEFINE_RANGE_PARAM(NAME, maxValue) /* ignored */
#include "plan_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
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
#include "plan_evaluation_feature.tab"
#undef DEFINE_PARAM
#undef DEFINE_RANGE_PARAM
    SIZE_OF_PLAN_RANGE_FEATURE_PARAM
};

inline PlanRangeFeatureParam toPlanRangeFeatureParam(int ith)
{
    DCHECK(0 <= ith && ith < SIZE_OF_PLAN_RANGE_FEATURE_PARAM);
    return static_cast<PlanRangeFeatureParam>(ith);
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

public:
    std::string toString() const;

public:
    double calculateScore(const EvaluationParams&) const;

private:
    // TODO(mayah): We would like to use std::array instead.
    std::vector<double> m_features;
    std::vector<int> m_rangeFeatures;
};

// TODO: Maybe we have to have bestEvaluationFeature in this class.
class EvaluationFeature {
public:
    const PlanEvaluationFeature& planFeature() const { return m_planFeature; }
    void setPlanFeature(const PlanEvaluationFeature& planFeature) { m_planFeature = planFeature; }

    const RensaEvaluationFeature& findBestRensaFeature(const EvaluationParams&) const;

    
    void add(const RensaEvaluationFeature& rensaEvaluationFeature) { m_rensaFeatures.push_back(rensaEvaluationFeature); }

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
    EvaluationParams();

    double get(PlanFeatureParam param) const { return m_planFeaturesCoef[param]; }
    void set(PlanFeatureParam param, double value) { m_planFeaturesCoef[param] = value; }
    void add(PlanFeatureParam param, double value) { m_planFeaturesCoef[param] += value; }

    double get(PlanRangeFeatureParam param, int index) const { return m_planRangeFeaturesCoef[param][index]; }
    void set(PlanRangeFeatureParam param, int i, double value) { m_planRangeFeaturesCoef[param][i] = value; }
    void set(PlanRangeFeatureParam param, const std::vector<double>& values) { m_planRangeFeaturesCoef[param] = values; }
    void add(PlanRangeFeatureParam param, int i, double value) { m_planRangeFeaturesCoef[param][i] += value; }

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

private:
    void initialize();
    
    std::vector<double> m_planFeaturesCoef;
    std::vector<std::vector<double> > m_planRangeFeaturesCoef;
    std::vector<double> m_rensaFeaturesCoef;
    std::vector<std::vector<double> > m_rensaRangeFeaturesCoef;
};

#endif
