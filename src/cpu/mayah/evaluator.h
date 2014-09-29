#ifndef EVALUATION_FEATURE_COLLECTOR_H_
#define EVALUATION_FEATURE_COLLECTOR_H_

#include <map>
#include <vector>

#include "book_field.h"
#include "feature_parameter.h"
#include "score_collector.h"

class CoreField;
class Gazer;
class RefPlan;

class EvalResult {
public:
    constexpr EvalResult(double score, int maxVirtualScore) : score_(score), maxVirtualScore_(maxVirtualScore) {}

    double score() const { return score_; }
    int maxVirtualScore() const { return maxVirtualScore_; }

private:
    double score_;
    int maxVirtualScore_;
};

class Evaluator {
public:
    Evaluator(const FeatureParameter& param, const std::vector<BookField>& books) : param_(param), books_(books) {}

    EvalResult eval(const RefPlan&, const CoreField& currentField, int currentFrameId, int maxIteration, const Gazer&);
    // Same as eval(), but returns CollectedFeature.
    CollectedFeature evalWithCollectingFeature(const RefPlan&, const CoreField& currentField, int currentFrameId,
                                               int maxIteration, const Gazer&);

private:
    const FeatureParameter& param_;
    const std::vector<BookField>& books_;
};

#include "evaluator_inl.h"

#endif
