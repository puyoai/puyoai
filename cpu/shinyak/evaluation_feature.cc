#include "evaluation_feature.h"

using namespace std;

string EvaluationFeature::toString() const
{
    return "NOT IMPLEMENTED YET";
}

double EvaluationFeature::calculateScore() const
{
    double result = 0;
    result += 1.0 * get(MAX_CHAINS);
    result += 1.0 / get(MAX_RENSA_NECESSARY_PUYOS);
    return result;
}
