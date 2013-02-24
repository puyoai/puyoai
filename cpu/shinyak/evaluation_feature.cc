#include "evaluation_feature.h"

using namespace std;

string EvaluationFeature::toString() const
{
    // TODO(mayah): We have to implement this.
    return "NOT IMPLEMENTED YET";
}

double EvaluationFeature::calculateScore() const
{
    double result = 0;
    result += 1.0 * get(MAX_CHAINS);
    result += 1.0 / get(MAX_RENSA_NECESSARY_PUYOS);

    if (get(THIRD_COLUMN_HEIGHT) >= 9)
        result -= 0.5;
    if (get(THIRD_COLUMN_HEIGHT) >= 10)
        result -= 1.5;

    result -= 0 * get(SUM_OF_HEIGHT_DIFF_FROM_AVERAGE);
    result -= 0.1 * get(SQUARE_SUM_OF_HEIGHT_DIFF_FROM_AVERAGE);

    result += get(EMPTY_AVAILABILITY_00) * 0.95;
    result += get(EMPTY_AVAILABILITY_01) * 0.90;
    result += get(EMPTY_AVAILABILITY_02) * 0.85;
    result += get(EMPTY_AVAILABILITY_11) * 0.80;
    result += get(EMPTY_AVAILABILITY_12) * 0.75;
    result += get(EMPTY_AVAILABILITY_22) * 0.30;
    
    return result;
}
