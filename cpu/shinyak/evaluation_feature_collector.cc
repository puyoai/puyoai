#include "evaluation_feature_collector.h"

#include <algorithm>
#include <cmath>
#include <glog/logging.h>

#include "core/constant.h"
#include "evaluation_feature.h"
#include "field_bit_field.h"
#include "plan.h"
#include "rensa_info.h"
#include "rensa_result.h"

using namespace std;

void EvaluationFeatureCollector::collectEmptyAvailabilityFeature(EvaluationFeature& feature, const Field& field)
{
    int emptyCells = 72 - field.countPuyos();
    if (emptyCells <= 0)
        return;

    DoubleFeatureParam map[3][3] = {
        { EMPTY_AVAILABILITY_00, EMPTY_AVAILABILITY_01, EMPTY_AVAILABILITY_02, },
        { EMPTY_AVAILABILITY_01, EMPTY_AVAILABILITY_11, EMPTY_AVAILABILITY_12, },
        { EMPTY_AVAILABILITY_02, EMPTY_AVAILABILITY_12, EMPTY_AVAILABILITY_22, },
    };

    for (int x = Field::WIDTH; x >= 1; --x) {
        for (int y = Field::HEIGHT; y >= 1; --y) {
            if (field.color(x, y) != EMPTY)
                continue;

            int left = 2, right = 2;
            if (field.color(x - 1, y) == EMPTY) --left;
            if (field.color(x - 1, y + 1) == EMPTY) --left;
            if (field.color(x + 1, y) == EMPTY) --right;
            if (field.color(x + 1, y + 1) == EMPTY) --right;

            feature.set(map[left][right], feature.get(map[left][right]) + 1.0 / emptyCells);
        }
    }
}

static void calculateConnection(const Field& field, const IntegerFeatureParam params[], EvaluationFeature& feature)
{
    FieldBitField checked;
    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = 1; field.color(x, y) != EMPTY; ++y) {
            if (!isColorPuyo(field.color(x, y)) || checked(x, y))
                continue;

            pair<int, int> connection = field.connectedPuyoNumsWithAllowingOnePointJump(x, y, checked);
            if (connection.first + connection.second >= 4)
                feature.add(params[3], 1);
            else if (connection.first < 4)
                feature.add(params[connection.first - 1], 1);
        }
    }
}

void EvaluationFeatureCollector::collectConnectionFeature(EvaluationFeature& feature, const Field& field, const TrackResult& trackResult)
{
    static const IntegerFeatureParam params[] = {
        CONNECTION_1,
        CONNECTION_2,
        CONNECTION_3,
        CONNECTION_4, 
    };

    static const IntegerFeatureParam paramsAfter[] = {
        CONNECTION_AFTER_VANISH_1,
        CONNECTION_AFTER_VANISH_2,
        CONNECTION_AFTER_VANISH_3,
        CONNECTION_AFTER_VANISH_4,
    };

    ArbitrarilyModifiableField f(field);
    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = 1; y <= 13; ++y) { // TODO: 13?
            if (trackResult.erasedAt(x, y) != 0)
                f.setColor(x, y, EMPTY);
        }
        f.recalcHeightOn(x);
    }
    f.forceDrop();

    calculateConnection(field, params, feature);
    calculateConnection(f, paramsAfter, feature);
}

void EvaluationFeatureCollector::collectFieldHeightFeature(EvaluationFeature& feature, const Field& field)
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
