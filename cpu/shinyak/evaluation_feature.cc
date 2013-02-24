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

    return result;
}
