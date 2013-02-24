#ifndef __FIELD_EVALUATOR_H_
#define __FIELD_EVALUATOR_H_

class EvaluationFeature;
class Field;
class TrackResult;

class FieldEvaluator {
public:
    static void calculateConnectionScore(const Field&, const TrackResult&, EvaluationFeature&);
    static void calculateEmptyFieldAvailability(const Field&, EvaluationFeature&);
    static void calculateFieldHeightScore(const Field&, EvaluationFeature&);
};

#endif
