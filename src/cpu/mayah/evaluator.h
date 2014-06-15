#ifndef EVALUATION_FEATURE_COLLECTOR_H_
#define EVALUATION_FEATURE_COLLECTOR_H_

#include "evaluation_feature.h"

class CoreField;
class Gazer;
class RefPlan;
struct TrackedPossibleRensaInfo;

struct EvalResult {
    EvalResult(double score, const std::string& message);

    double evaluationScore;
    std::string message;
};

class Evaluator {
public:
    static const int NUM_KEY_PUYOS = 1;

    explicit Evaluator(const EvaluationFeature& feature) : feature_(feature) {}

    EvalResult eval(const RefPlan&, int currentFrameId, const Gazer&);

private:
    double evalFrameFeature(const RefPlan&);
    double evalConnectionFeature(const RefPlan&);
    double evalDensityFeature(const RefPlan&);
    double evalPuyoPattern33Feature(const RefPlan&);
    double evalFieldHeightFeature(const RefPlan&);
    double evalThirdColumnHeightFeature(const RefPlan&);
    double evalEmptyAvailabilityFeature(const RefPlan&);
    double evalOngoingRensaFeature(const RefPlan&, int currentFrameId, const Gazer&);

    // Methods to eval rensa features.
    double evalRensaChainFeature(const RefPlan&, const TrackedPossibleRensaInfo&);
    double evalRensaHandWidthFeature(const RefPlan&, const TrackedPossibleRensaInfo&);
    double evalRensaConnectionFeature(const RefPlan&, const CoreField& fieldAfterDrop);
    double evalRensaGarbageFeature(const RefPlan&, const CoreField& fieldAfterDrop);

    EvaluationFeature feature_;
};

#endif
