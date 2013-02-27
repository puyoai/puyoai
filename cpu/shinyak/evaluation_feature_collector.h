#ifndef __EVALUATION_FEATURE_COLLECTOR_H_
#define __EVALUATION_FEATURE_COLLECTOR_H_

class EvaluationFeature;
class Field;
class MyPlayerInfo;
class TrackResult;
class Plan;

class EvaluationFeatureCollector {
public:
    // Collects all features.
    static void collectFeatures(EvaluationFeature&, const Plan&, const MyPlayerInfo&);

public:
    static void collectMaxRensaFeature(EvaluationFeature&, const Field&);
    static void collectConnectionFeature(EvaluationFeature&, const Field&, const TrackResult&);
    static void collectEmptyAvailabilityFeature(EvaluationFeature&, const Field&);
    static void collectFieldHeightFeature(EvaluationFeature&, const Field&);
    static void collectMainRensaHandWidth(EvaluationFeature&, const MyPlayerInfo&);
};

#endif
