#ifndef EVALUATION_FEATURE_COLLECTOR_H_
#define EVALUATION_FEATURE_COLLECTOR_H_

class CoreField;
class EnemyInfo;
class EvaluationFeature;
class RensaTrackResult;
class PlanEvaluationFeature;
class RefPlan;
class RensaEvaluationFeature;
struct TrackedPossibleRensaInfo;

class EvaluationFeatureCollector {
public:
    // Collects all features.
    static void collectFeatures(EvaluationFeature&, const RefPlan&, int numKeyPuyos, int currentFrameId, const EnemyInfo&);

private:
    static void collectPlanFeatures(PlanEvaluationFeature&, const RefPlan&, int currentFrameId, const EnemyInfo&);

    // Methods to collect plan features.
    static void collectFrameFeature(PlanEvaluationFeature&, const RefPlan&);
    static void collectConnectionFeature(PlanEvaluationFeature&, const RefPlan&);
    static void collectDensityFeature(PlanEvaluationFeature&, const RefPlan&);
    static void collectPuyoPattern33Feature(PlanEvaluationFeature&, const RefPlan&);
    static void collectFieldHeightFeature(PlanEvaluationFeature&, const RefPlan&);
    static void collectEmptyAvailabilityFeature(PlanEvaluationFeature&, const RefPlan&);
    static void collectOngoingRensaFeature(PlanEvaluationFeature&, const RefPlan&, int currentFrameId, const EnemyInfo&);

private:
    static void collectRensaFeatures(RensaEvaluationFeature&, const RefPlan&, const TrackedPossibleRensaInfo&);

    // Methods to collect rensa features.
    static void collectRensaChainFeature(RensaEvaluationFeature&, const RefPlan&, const TrackedPossibleRensaInfo&);
    static void collectRensaHandWidthFeature(RensaEvaluationFeature&, const RefPlan&, const TrackedPossibleRensaInfo&);
    static void collectRensaConnectionFeature(RensaEvaluationFeature&, const CoreField& fieldAfterRensa, const CoreField& fieldAfterDrop);
    static void collectRensaGarbageFeature(RensaEvaluationFeature&, const RefPlan&, const CoreField& fieldAfterDrop);
};

#endif
