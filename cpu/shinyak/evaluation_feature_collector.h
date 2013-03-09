#ifndef __EVALUATION_FEATURE_COLLECTOR_H_
#define __EVALUATION_FEATURE_COLLECTOR_H_

class EnemyInfo;
class EvaluationFeature;
class Field;
class MyPlayerInfo;
class TrackResult;
class Plan;
class PlanEvaluationFeature;
class RensaEvaluationFeature;
struct TrackedPossibleRensaInfo;

class EvaluationFeatureCollector {
public:
    // Collects all features.
    static void collectFeatures(EvaluationFeature&, const Plan&, int currentFrameId, const EnemyInfo&);

    static void collectPlanFeature(EvaluationFeature&, const Plan&, int currentFrameId, const EnemyInfo&);
    static void collectRensaFeature(EvaluationFeature&, const Plan&);

private:
    // Methods to collect plan features.
    static void collectFrameFeature(PlanEvaluationFeature&, const Plan&);
    static void collectConnectionFeature(PlanEvaluationFeature&, const Plan&);
    static void collectDensityFeature(PlanEvaluationFeature&, const Plan&);
    static void collectFieldHeightFeature(PlanEvaluationFeature&, const Plan&);
    static void collectEmptyAvailabilityFeature(PlanEvaluationFeature&, const Plan&);
    static void collectOngoingRensaFeature(PlanEvaluationFeature&, const Plan&, int currentFrameId, const EnemyInfo&);

private:
    // Methods to collect rensa features.
    static void collectRensaEvaluationFeature(RensaEvaluationFeature&, const Plan&, const TrackedPossibleRensaInfo&);
};

#endif
