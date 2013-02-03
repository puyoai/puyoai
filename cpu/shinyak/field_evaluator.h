#ifndef __FIELD_EVALUATOR_H_
#define __FIELD_EVALUATOR_H_

#include <stdio.h>
#include <vector>
#include "field.h"
#include "player_info.h"

// TODO(mayah): non evaluation function should be move to Field.
class FieldEvaluator {
public:
    static double calculateEmptyFieldAvailability(const Field&);
    static double calculateConnectionScore(const Field&);
    static double calculateFieldHeightScore(const Field&);
};

#endif
