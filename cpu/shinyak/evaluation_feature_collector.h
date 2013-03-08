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
    static void collectFeatures(EvaluationFeature&, int currentFrameId, const Plan&, const EnemyInfo&);

public:
    static void collectOngoingRensaFeature(PlanEvaluationFeature&, int currentFrameId, const Plan&, const EnemyInfo&);

private:
    static void collectFrameFeature(PlanEvaluationFeature&, const Plan&);
    static void collectConnectionFeature(PlanEvaluationFeature&, const Plan&);
    static void collectFieldHeightFeature(PlanEvaluationFeature&, const Plan&);
    static void collectEmptyAvailabilityFeature(PlanEvaluationFeature&, const Plan&);

private:
    static void collectRensaFeature(EvaluationFeature&, const Plan&);
    static void collectRensaEvaluationFeature(RensaEvaluationFeature&, const Plan&, const TrackedPossibleRensaInfo&);
};

#endif
