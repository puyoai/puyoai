#ifndef __EVALUATION_FEATURE_COLLECTOR_H_
#define __EVALUATION_FEATURE_COLLECTOR_H_

class EvaluationFeature;
class Field;
class TrackResult;

class EvaluationFeatureCollector {
public:
    static void collectConnectionFeature(EvaluationFeature&, const Field&, const TrackResult&);
    static void collectEmptyAvailabilityFeature(EvaluationFeature&, const Field&);
    static void collectFieldHeightFeature(EvaluationFeature&, const Field&);
};

#endif
