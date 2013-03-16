#include "evaluation_feature_collector.h"

#include <algorithm>
#include <cmath>
#include <glog/logging.h>
#include <sstream>

#include "core/constant.h"
#include "enemy_info.h"
#include "evaluation_feature.h"
#include "field.h"
#include "field_bit_field.h"
#include "plan.h"
#include "player_info.h"
#include "puyo_possibility.h"
#include "rensa_detector.h"
#include "rensa_info.h"
#include "rensa_result.h"
#include "score.h"

using namespace std;

void EvaluationFeatureCollector::collectFeatures(EvaluationFeature& feature, const Plan& plan,
                                                 int numKeyPuyos, int currentFrameId, const EnemyInfo& enemyInfo)
{
    collectPlanFeatures(feature.modifiablePlanFeature(), plan, currentFrameId, enemyInfo);

    const Field& field = plan.field();

    vector<vector<TrackedPossibleRensaInfo>> rensaInfos;
    RensaDetector::findPossibleRensasUsingIteration(rensaInfos, field, numKeyPuyos);
    
    if (rensaInfos.empty())
        return;

    for (int i = 0; i <= numKeyPuyos; ++i) {
        // Collects only chains having the maxChains.
        // TODO(mayah): Is it desired? How about performance if we collect all rensas?
        int maxChains = 0;
        for (auto it = rensaInfos[i].begin(); it != rensaInfos[i].end(); ++it)
            maxChains = std::max(maxChains, it->rensaInfo.chains);

        for (auto it = rensaInfos[i].begin(); it != rensaInfos[i].end(); ++it) {
            if (it->rensaInfo.chains < maxChains)
                continue;

            RensaEvaluationFeature rensaFeature;
            collectRensaFeatures(rensaFeature, plan, *it);

            feature.addRensaFeature(rensaFeature);
            // TODO(mayah): If we have a lot of rensa features, it's too slow.
            // So we limit the number of rensa features.
            if (feature.numRensaFeatures() > 100)
                break;
        }
    }
}

void EvaluationFeatureCollector::collectPlanFeatures(PlanEvaluationFeature& planFeature, const Plan& plan, int currentFrameId, const EnemyInfo& enemyInfo)
{
    collectFrameFeature(planFeature, plan);
    collectEmptyAvailabilityFeature(planFeature, plan);
    collectConnectionFeature(planFeature, plan);
    collectDensityFeature(planFeature, plan);
    collectPuyoPattern33Feature(planFeature, plan);
    collectFieldHeightFeature(planFeature, plan);
    collectOngoingRensaFeature(planFeature, plan, currentFrameId, enemyInfo);
}

void EvaluationFeatureCollector::collectFrameFeature(PlanEvaluationFeature& planFeature, const Plan& plan)
{
    // TODO(mayah): Why totalFrames is 0?
    planFeature.set(TOTAL_FRAMES, plan.totalFrames());
    if (plan.totalFrames() != 0)
        planFeature.set(TOTAL_FRAMES_INVERSE, 1.0 / plan.totalFrames());    
}

template<typename Feature, typename T>
static void calculateConnection(Feature& feature, const Field& field, const T params[])
{
    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = 1; field.color(x, y) != EMPTY; ++y) {
            if (!isColorPuyo(field.color(x, y)))
                continue;

            int numConnected = field.connectedPuyoNums(x, y);
            DCHECK(1 <= numConnected && numConnected <= 3);
            feature.add(params[numConnected - 1], 1);
        }
    }
}

template<typename Feature, typename T>
static void calculateConnectionWithAllowingOnePointJump(Feature& feature, const Field& field, const T params[])
{
    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = 1; field.color(x, y) != EMPTY; ++y) {
            if (!isColorPuyo(field.color(x, y)))
                continue;

            pair<int, int> connection = field.connectedPuyoNumsWithAllowingOnePointJump(x, y);
            if (connection.first + connection.second >= 4)
                feature.add(params[3], 1);
            else if (connection.first < 4)
                feature.add(params[connection.first - 1], 1);
        }
    }
}

// Takes 2x3 field, and counts each color puyo number.
template<typename Feature, typename T>
static void calculateDensity(Feature& feature, const Field& field, const T params[])
{
    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = 1; y <= Field::HEIGHT + 1; ++y) {
            int numColors[8] = { 0 };

            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    numColors[field.color(x + dx, y + dy)] += 1;
                }
            }

            for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
                PuyoColor c = normalPuyoColorOf(i);
                if (numColors[c] > 4)
                    numColors[c] = 4;
                feature.add(params[numColors[c]], 1);
            }
        }
    }
}

void EvaluationFeatureCollector::collectEmptyAvailabilityFeature(PlanEvaluationFeature& feature, const Plan& plan)
{
    const Field& field = plan.field();

    int emptyCells = 72 - field.countPuyos();
    if (emptyCells <= 0)
        return;

    PlanFeatureParam map[3][3] = {
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

void EvaluationFeatureCollector::collectConnectionFeature(PlanEvaluationFeature& planFeature, const Plan& plan)
{
    static const PlanFeatureParam params[] = {
        CONNECTION_1, CONNECTION_2, CONNECTION_3,
    };
    static const PlanFeatureParam jumpParams[] = {
        CONNECTION_ALLOWING_JUMP_1, CONNECTION_ALLOWING_JUMP_2, CONNECTION_ALLOWING_JUMP_3, CONNECTION_ALLOWING_JUMP_4,
    };

    calculateConnection(planFeature, plan.field(), params);
    calculateConnectionWithAllowingOnePointJump(planFeature, plan.field(), jumpParams);
}

void EvaluationFeatureCollector::collectDensityFeature(PlanEvaluationFeature& planFeature, const Plan& plan)
{
    static const PlanFeatureParam params[] = {
        DENSITY_0, DENSITY_1, DENSITY_2, DENSITY_3, DENSITY_4, 
    };

    calculateDensity(planFeature, plan.field(), params);
}

void EvaluationFeatureCollector::collectPuyoPattern33Feature(PlanEvaluationFeature& planFeature, const Plan& plan)
{
    const Field& field = plan.field();

    {
        int x = 2;
        int y = 2;
        int patterns[8] = { 0 };

        patterns[field.color(x - 1, y - 1)] |= 1 << 0;
        patterns[field.color(x    , y - 1)] |= 1 << 1;
        patterns[field.color(x + 1, y - 1)] |= 1 << 2;
        
        patterns[field.color(x - 1, y    )] |= 1 << 3;
        patterns[field.color(x    , y    )] |= 1 << 4;
        patterns[field.color(x + 1, y    )] |= 1 << 5;

        patterns[field.color(x - 1, y + 1)] |= 1 << 6;
        patterns[field.color(x    , y + 1)] |= 1 << 7;
        patterns[field.color(x + 1, y + 1)] |= 1 << 8;

        planFeature.add(PUYO_PATTERN_33, patterns[RED]);
        planFeature.add(PUYO_PATTERN_33, patterns[GREEN]);
        planFeature.add(PUYO_PATTERN_33, patterns[YELLOW]);
        planFeature.add(PUYO_PATTERN_33, patterns[BLUE]);
    }

    {
        int x = 5;
        int y = 2;
        int patterns[8] = { 0 };

        patterns[field.color(x + 1, y - 1)] |= 1 << 0;
        patterns[field.color(x    , y - 1)] |= 1 << 1;
        patterns[field.color(x - 1, y - 1)] |= 1 << 2;
        
        patterns[field.color(x + 1, y    )] |= 1 << 3;
        patterns[field.color(x    , y    )] |= 1 << 4;
        patterns[field.color(x - 1, y    )] |= 1 << 5;

        patterns[field.color(x + 1, y + 1)] |= 1 << 6;
        patterns[field.color(x    , y + 1)] |= 1 << 7;
        patterns[field.color(x - 1, y + 1)] |= 1 << 8;

        planFeature.add(PUYO_PATTERN_33, patterns[RED]);
        planFeature.add(PUYO_PATTERN_33, patterns[GREEN]);
        planFeature.add(PUYO_PATTERN_33, patterns[YELLOW]);
        planFeature.add(PUYO_PATTERN_33, patterns[BLUE]);
    }

}

void EvaluationFeatureCollector::collectFieldHeightFeature(PlanEvaluationFeature& planFeature, const Plan& plan)
{
    const Field& field = plan.field();
    
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

    planFeature.set(THIRD_COLUMN_HEIGHT, field.height(3));
    planFeature.set(SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSum);
    planFeature.set(SQUARE_SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSquareSum);
}

void EvaluationFeatureCollector::collectOngoingRensaFeature(PlanEvaluationFeature& planFeature, const Plan& plan, int currentFrameId, const EnemyInfo& enemyInfo)
{
    if (enemyInfo.rensaIsOngoing() && enemyInfo.ongoingRensaInfo().rensaInfo.score > scoreForOjama(6)) {
        // TODO: 対応が適当すぎる
        if (enemyInfo.ongoingRensaInfo().rensaInfo.score >= scoreForOjama(6) &&
            plan.totalScore() >= enemyInfo.ongoingRensaInfo().rensaInfo.score &&
            plan.initiatingFrames() <= enemyInfo.ongoingRensaInfo().finishingRensaFrame) {
            LOG(INFO) << plan.decisionText() << " TAIOU";
            planFeature.set(STRATEGY_TAIOU, 1.0);
            return;
        }
    }

    if (!plan.isRensaPlan())
        return;

    if (plan.field().isZenkeshi()) {
        planFeature.set(STRATEGY_ZENKESHI, 1);
        return;
    }

    int rensaEndingFrameId = currentFrameId + plan.totalFrames();
    int estimatedMaxScore = enemyInfo.estimateMaxScore(rensaEndingFrameId);

    // --- 1.1. 十分でかい場合は打って良い。
    // / TODO: 十分でかいとは？ / とりあえず致死量ということにする
    if (plan.totalScore() >= estimatedMaxScore + scoreForOjama(60)) {
        planFeature.set(STRATEGY_LARGE_ENOUGH, 1);
        return;
    }
        
    // --- 1.2. 対応手なく潰せる
    // TODO: 実装があやしい。
    if (plan.totalScore() >= scoreForOjama(18) && estimatedMaxScore <= scoreForOjama(6)) {
        planFeature.set(STRATEGY_TSUBUSHI, 1);
        return;
    }
        
    // --- 1.3. 飽和したので打つしかなくなった
    // TODO: これは EnemyRensaInfo だけじゃなくて MyRensaInfo も必要なのでは……。
    // TODO: 60 個超えたら打つとかなんか間違ってるだろう。
    if (plan.field().countPuyos() >= 60) {
        planFeature.set(STRATEGY_HOUWA, 1);
    }
    
    // --- 1.4. 打つと有利になる
    // TODO: そもそも数値化の仕方が分からない。
    
    // 基本的に先打ちすると負けるので、打たないようにする
    ostringstream ss;
    ss << "SAKIUCHI will lose : score = " << plan.totalScore() << " EMEMY score = " << estimatedMaxScore << endl;
    LOG(INFO) << plan.decisionText() << " " << ss.str();
    planFeature.set(STRATEGY_SCORE, plan.totalScore());
    planFeature.set(STRATEGY_SAKIUCHI, 1.0);
}

void EvaluationFeatureCollector::collectRensaFeatures(RensaEvaluationFeature& rensaFeature, const Plan& plan, const TrackedPossibleRensaInfo& info)    
{
    ArbitrarilyModifiableField fieldAfterRensa(plan.field());
    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = 1; y <= 13; ++y) { // TODO: 13?
            if (info.trackResult.erasedAt(x, y) != 0)
                fieldAfterRensa.setColor(x, y, EMPTY);
        }
        fieldAfterRensa.recalcHeightOn(x);
    }

    ArbitrarilyModifiableField fieldAfterDrop(fieldAfterRensa);
    fieldAfterDrop.forceDrop();

    collectRensaChainFeature(rensaFeature, plan, info);
    collectRensaHandWidthFeature(rensaFeature, plan, info);
    collectRensaConnectionFeature(rensaFeature, fieldAfterRensa, fieldAfterDrop);
    collectRensaGarbageFeature(rensaFeature, plan, fieldAfterDrop);
}

void EvaluationFeatureCollector::collectRensaChainFeature(RensaEvaluationFeature& rensaFeature, const Plan& /*plan*/, const TrackedPossibleRensaInfo& info)
{
    int numNecessaryPuyos = TsumoPossibility::necessaryPuyos(0.5, info.necessaryPuyoSet);
    
    rensaFeature.set(MAX_CHAINS, info.rensaInfo.chains);
    rensaFeature.set(MAX_RENSA_NECESSARY_PUYOS, numNecessaryPuyos);
    if (numNecessaryPuyos != 0)
        rensaFeature.set(MAX_RENSA_NECESSARY_PUYOS_INVERSE, 1.0 / numNecessaryPuyos);
}

void EvaluationFeatureCollector::collectRensaHandWidthFeature(RensaEvaluationFeature& rensaFeature, const Plan& plan, const TrackedPossibleRensaInfo& info)
{
    // -----
    int distanceCountResult[5] = { 0, 0, 0, 0, 0 };

    // 1 連鎖の部分を距離 1 とし、距離 4 までを求める。
    // 距離 2, 3, 4 の数を数え、その広がり具合により、手の広さを求めることができる。
    int distance[Field::MAP_WIDTH][Field::MAP_HEIGHT];
    for (int x = 0; x < Field::MAP_WIDTH; ++x) {
        for (int y = 0; y < Field::MAP_HEIGHT; ++y) {
            if (info.trackResult.erasedAt(x, y) == 1)
                distance[x][y] = 1;
            else
                distance[x][y] = 0;
        }
    }

    const Field& field = plan.field();
    for (int d = 2; d <= 4; ++d) {
        for (int x = 1; x <= Field::WIDTH; ++x) {
            for (int y = 1; y <= Field::HEIGHT; ++y) {
                if (field.color(x, y) != EMPTY || distance[x][y] > 0)
                    continue;
                if (distance[x][y-1] == d - 1 || distance[x][y+1] == d - 1 || distance[x-1][y] == d - 1 || distance[x+1][y] == d - 1) {
                    distance[x][y] = d;
                    ++distanceCountResult[d];
                }
            }
        }
    }

    double d2 = distanceCountResult[2];
    double d3 = distanceCountResult[3];
    double d4 = distanceCountResult[4];

    double r32 = d2 != 0 ? d3 / d2 : 0;
    double r43 = d3 != 0 ? d4 / d3 : 0;

    rensaFeature.set(HAND_WIDTH_2, d2);
    rensaFeature.set(HAND_WIDTH_3, d3);
    rensaFeature.set(HAND_WIDTH_4, d4);
    rensaFeature.set(HAND_WIDTH_RATIO_32, r32);
    rensaFeature.set(HAND_WIDTH_RATIO_43, r43);
    rensaFeature.set(HAND_WIDTH_RATIO_32_SQUARED, r32 * r32);
    rensaFeature.set(HAND_WIDTH_RATIO_43_SQUARED, r43 * r43);
}

void EvaluationFeatureCollector::collectRensaConnectionFeature(RensaEvaluationFeature& rensaFeature, const Field& /*fieldAfterRensa*/, const Field& fieldAfterDrop)
{
    static const RensaFeatureParam paramsAfter[] = {
        CONNECTION_AFTER_VANISH_1, CONNECTION_AFTER_VANISH_2, CONNECTION_AFTER_VANISH_3,
    };
    static const RensaFeatureParam paramsJumpAfter[] = {
        CONNECTION_ALLOWING_JUMP_AFTER_VANISH_1, CONNECTION_ALLOWING_JUMP_AFTER_VANISH_2,
        CONNECTION_ALLOWING_JUMP_AFTER_VANISH_3, CONNECTION_ALLOWING_JUMP_AFTER_VANISH_4,
    };

    calculateConnection(rensaFeature, fieldAfterDrop, paramsAfter);
    calculateConnectionWithAllowingOnePointJump(rensaFeature, fieldAfterDrop, paramsJumpAfter);
}

void EvaluationFeatureCollector::collectRensaGarbageFeature(RensaEvaluationFeature& rensaFeature, const Plan& plan, const Field& fieldAfterDrop)
{
    rensaFeature.set(NUM_GARBAGE_PUYOS, plan.field().countPuyos() - fieldAfterDrop.countPuyos());
}
