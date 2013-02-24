#include "evaluation_feature.h"

#include <sstream>

using namespace std;

string EvaluationFeature::toString() const
{
    ostringstream ss;
    ss << get(MAX_CHAINS) << " ";
    ss << (1.0 / get(MAX_RENSA_NECESSARY_PUYOS)) << " ";
    // TODO(mayah): We have to implement this.

    return ss.str();
}

double EvaluationFeature::calculateHandWidthScore(int /*numFirstCells*/, int numSecondCells, int numThirdCells, int numFourthCells)
{
    if (numSecondCells == 0 || numThirdCells == 0)
        return 0;

    double ratio3 = static_cast<double>(numFourthCells) / numThirdCells;
    double ratio2 = static_cast<double>(numThirdCells) / numSecondCells;

    double r2 = (ratio2 - 1.5) * (ratio2 - 1.5) * (ratio2 - 1.5 >- 0 ? 1 : -1);
    double r3 = (ratio3 - 1.5) * (ratio3 - 1.5) * (ratio3 - 1.5 >- 0 ? 1 : -1);

    return r2 + r3;
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

    result += -0.1 * get(CONNECTION_1) / 30.0;
    result +=  0.9 * get(CONNECTION_2) / 30.0;
    result +=  1.2 * get(CONNECTION_3) / 30.0;
    result += -0.5 * get(CONNECTION_4) / 30.0; // minus
    result += -0.1 * get(CONNECTION_AFTER_VANISH_1) / 15.0;
    result +=  0.9 * get(CONNECTION_AFTER_VANISH_2) / 15.0;
    result +=  1.6 * get(CONNECTION_AFTER_VANISH_3) / 15.0;
    result +=  0.5 * get(CONNECTION_AFTER_VANISH_4) / 15.0; // plus

    result += calculateHandWidthScore(get(HAND_WIDTH_1), get(HAND_WIDTH_2), get(HAND_WIDTH_3), get(HAND_WIDTH_4));
    result += 1.0 / get(TOTAL_FRAMES);
    
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
