#ifndef __FIELD_EVALUATOR_H_
#define __FIELD_EVALUATOR_H_

class EvaluationFeature;
class Field;

class FieldEvaluator {
public:
    static double calculateEmptyFieldAvailability(const Field&);
    static double calculateConnectionScore(const Field&);

    static void calculateFieldHeightScore(const Field&, EvaluationFeature&);
};

#endif
