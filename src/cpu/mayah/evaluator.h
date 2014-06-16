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

    Evaluator() {}

    EvalResult eval(const EvaluationFeature&, const RefPlan&, int currentFrameId, const Gazer&);

private:
    double evalFrameFeature(const EvaluationFeature&, const RefPlan&);
    double evalConnectionFeature(const EvaluationFeature&, const RefPlan&);
    double evalDensityFeature(const EvaluationFeature&, const RefPlan&);
    double evalPuyoPattern33Feature(const EvaluationFeature&, const RefPlan&);
    double evalFieldHeightFeature(const EvaluationFeature&, const RefPlan&);
    double evalThirdColumnHeightFeature(const EvaluationFeature&, const RefPlan&);
    double evalEmptyAvailabilityFeature(const EvaluationFeature&, const RefPlan&);
    double evalOngoingRensaFeature(const EvaluationFeature&, const RefPlan&, int currentFrameId, const Gazer&);

    // Methods to eval rensa features.
    double evalRensaChainFeature(const EvaluationFeature&, const RefPlan&, const TrackedPossibleRensaInfo&);
    double evalRensaHandWidthFeature(const EvaluationFeature&, const RefPlan&, const TrackedPossibleRensaInfo&);
    double evalRensaConnectionFeature(const EvaluationFeature&, const RefPlan&, const CoreField& fieldAfterDrop);
    double evalRensaGarbageFeature(const EvaluationFeature&, const RefPlan&, const CoreField& fieldAfterDrop);
};

#endif
