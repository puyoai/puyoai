#include "evaluation_feature_collector.h"

#include <algorithm>
#include <cmath>
#include <glog/logging.h>

#include "core/constant.h"
#include "evaluation_feature.h"
#include "field_bit_field.h"
#include "plan.h"
#include "player_info.h"
#include "puyo_possibility.h"
#include "rensa_detector.h"
#include "rensa_info.h"
#include "rensa_result.h"

using namespace std;

void EvaluationFeatureCollector::collectMaxRensaFeature(EvaluationFeature& feature, const Field& field)
{
    int maxChains = 0;
    int numNecessaryPuyos = 0;

    vector<PossibleRensaInfo> rensaInfos;
    RensaDetector::findPossibleRensas(rensaInfos, field);
    for (vector<PossibleRensaInfo>::iterator it = rensaInfos.begin(); it != rensaInfos.end(); ++it) {
        if (maxChains < it->rensaInfo.chains) {
            maxChains = it->rensaInfo.chains;
            numNecessaryPuyos = TsumoPossibility::necessaryPuyos(0.5, it->necessaryPuyoSet);
        } else if (maxChains == it->rensaInfo.chains) {
            numNecessaryPuyos = min(numNecessaryPuyos, TsumoPossibility::necessaryPuyos(0.5, it->necessaryPuyoSet));
        }
    }
    
    feature.set(EvaluationFeature::MAX_CHAINS, maxChains);
    feature.set(EvaluationFeature::MAX_RENSA_NECESSARY_PUYOS, numNecessaryPuyos);
    feature.set(EvaluationFeature::MAX_RENSA_NECESSARY_PUYOS_INVERSE, 1.0 / numNecessaryPuyos);
}

void EvaluationFeatureCollector::collectEmptyAvailabilityFeature(EvaluationFeature& feature, const Field& field)
{
    int emptyCells = 72 - field.countPuyos();
    if (emptyCells <= 0)
        return;

    EvaluationFeature::FeatureParam map[3][3] = {
        { EvaluationFeature::EMPTY_AVAILABILITY_00, EvaluationFeature::EMPTY_AVAILABILITY_01, EvaluationFeature::EMPTY_AVAILABILITY_02, },
        { EvaluationFeature::EMPTY_AVAILABILITY_01, EvaluationFeature::EMPTY_AVAILABILITY_11, EvaluationFeature::EMPTY_AVAILABILITY_12, },
        { EvaluationFeature::EMPTY_AVAILABILITY_02, EvaluationFeature::EMPTY_AVAILABILITY_12, EvaluationFeature::EMPTY_AVAILABILITY_22, },
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

static void calculateConnection(const Field& field, const EvaluationFeature::FeatureParam params[], EvaluationFeature& feature)
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
    static const EvaluationFeature::FeatureParam params[] = {
        EvaluationFeature::CONNECTION_1,
        EvaluationFeature::CONNECTION_2,
        EvaluationFeature::CONNECTION_3,
        EvaluationFeature::CONNECTION_4, 
    };

    static const EvaluationFeature::FeatureParam paramsAfter[] = {
        EvaluationFeature::CONNECTION_AFTER_VANISH_1,
        EvaluationFeature::CONNECTION_AFTER_VANISH_2,
        EvaluationFeature::CONNECTION_AFTER_VANISH_3,
        EvaluationFeature::CONNECTION_AFTER_VANISH_4,
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

    feature.set(EvaluationFeature::THIRD_COLUMN_HEIGHT, field.height(3));
    feature.set(EvaluationFeature::SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSum);
    feature.set(EvaluationFeature::SQUARE_SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSquareSum);
}

void EvaluationFeatureCollector::collectMainRensaHandWidth(EvaluationFeature& feature, const MyPlayerInfo& playerInfo)
{
    double d1 = playerInfo.mainRensaDistanceCount(1);
    double d2 = playerInfo.mainRensaDistanceCount(2);
    double d3 = playerInfo.mainRensaDistanceCount(3);
    double d4 = playerInfo.mainRensaDistanceCount(4);

    double r21 = d1 != 0 ? d2 / d1 : 0;
    double r32 = d2 != 0 ? d3 / d2 : 0;
    double r43 = d3 != 0 ? d4 / d3 : 0;

    feature.set(EvaluationFeature::HAND_WIDTH_1, d1);
    feature.set(EvaluationFeature::HAND_WIDTH_2, d2);
    feature.set(EvaluationFeature::HAND_WIDTH_3, d3);
    feature.set(EvaluationFeature::HAND_WIDTH_4, d4);
    feature.set(EvaluationFeature::HAND_WIDTH_RATIO_21, r21);
    feature.set(EvaluationFeature::HAND_WIDTH_RATIO_32, r32);
    feature.set(EvaluationFeature::HAND_WIDTH_RATIO_43, r43);
    feature.set(EvaluationFeature::HAND_WIDTH_RATIO_21_SQUARED, r21 * r21);
    feature.set(EvaluationFeature::HAND_WIDTH_RATIO_32_SQUARED, r32 * r32);
    feature.set(EvaluationFeature::HAND_WIDTH_RATIO_43_SQUARED, r43 * r43);
}

