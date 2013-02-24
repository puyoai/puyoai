#include "field_evaluator.h"

#include <algorithm>
#include <cmath>
#include <glog/logging.h>

#include "../../core/constant.h"
#include "evaluation_feature.h"
#include "field_bit_field.h"
#include "plan.h"
#include "rensa_info.h"

using namespace std;

double FieldEvaluator::calculateEmptyFieldAvailability(const Field& field)
{
    // --- 1. Calculate the availability of field.
    static const double availabilityMap[3][3] = {
        { 0.95, 0.90, 0.85 },
        { 0.90, 0.80, 0.75 },
        { 0.85, 0.75, 0.30 },
    };

    double emptyAvailability = 0;
    for (int x = Field::WIDTH; x >= 1; --x) {
        for (int y = Field::HEIGHT; y >= 1; --y) {
            if (field.color(x, y) != EMPTY)
                continue;

            int left = 2, right = 2;
            if (field.color(x - 1, y) == EMPTY) --left;
            if (field.color(x - 1, y + 1) == EMPTY) --left;
            if (field.color(x + 1, y) == EMPTY) --right;
            if (field.color(x + 1, y + 1) == EMPTY) --right;
            
            emptyAvailability += availabilityMap[left][right];
        }
    }

    return emptyAvailability;
}

double FieldEvaluator::calculateConnectionScore(const Field& field)
{
    static double connectionScore[] = { 0, 0, 0.9, 1.2 };

    FieldBitField checked;
    double result = 0;
    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = 1; field.color(x, y) != EMPTY; ++y) {
            if (!isColorPuyo(field.color(x, y)) || checked(x, y))
                continue;

            pair<int, int> connection = field.connectedPuyoNumsWithAllowingOnePointJump(x, y, checked);
            if (connection.first + connection.second >= 4)
                result -= 0.5;
            else if (connection.first < 4)
                result += connectionScore[connection.first];
        }
    }

    return result;
}

void FieldEvaluator::calculateFieldHeightScore(const Field& field, EvaluationFeature& feature)
{
    double sumHeight = 0;
    for (int x = 1; x < Field::WIDTH; ++x)
        sumHeight += field.height(x);
    double averageHeight = sumHeight / 6.0;

    double heightSum = 0.0;
    double heightSquareSum = 0.0;
    for (int x = 1; x <= Field::WIDTH; ++x) {
        double diff = abs(field.height(x) - averageHeight);
        heightSum += diff;
        heightSquareSum += diff * diff;
    }

    feature.set(THIRD_COLUMN_HEIGHT, field.height(3));
    feature.set(SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSum);
    feature.set(SQUARE_SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSquareSum);
}
