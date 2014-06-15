#include "evaluator.h"

#include <algorithm>
#include <cmath>
#include <glog/logging.h>
#include <sstream>

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/algorithm/rensa_info.h"
#include "core/algorithm/rensa_detector.h"
#include "core/constant.h"
#include "core/decision.h"
#include "core/field/core_field.h"
#include "core/field/field_bit_field.h"
#include "core/field/rensa_result.h"
#include "core/score.h"

#include "evaluation_feature.h"
#include "gazer.h"

using namespace std;

EvalResult::EvalResult(double score, const string& message) :
    evaluationScore(score),
    message(message)
{
    // TODO(mayah): Do we need convert here? Why not do in core/client?
    for (string::size_type i = 0; i < this->message.size(); ++i) {
        if (this->message[i] == ' ')
            this->message[i] = '_';
    }
}

EvalResult Evaluator::eval(const RefPlan& plan, int currentFrameId, const Gazer& gazer)
{
    double score = 0.0;
    score += evalFrameFeature(plan);
#if USE_EMPTY_AVAILABILITY_FEATURE
    score += evalEmptyAvailabilityFeature(plan);
#endif
#if USE_CONNECTION_FEATURE
    score += evalConnectionFeature(plan);
#endif
    score += evalDensityFeature(plan);
    score += evalPuyoPattern33Feature(plan);
    score += evalFieldHeightFeature(plan);
#if USE_THIRD_COLUMN_HEIGHT_FEATURE
    score += evalThirdColumnHeightFeature(plan);
#endif
    score += evalOngoingRensaFeature(plan, currentFrameId, gazer);

    vector<TrackedPossibleRensaInfo> rensaInfos =
        RensaDetector::findPossibleRensasWithTracking(plan.field(), NUM_KEY_PUYOS);
    double maxRensaScore = 0;
    for (size_t i = 0; i < 100 && i < rensaInfos.size(); ++i) {
        const auto& rensaInfo = rensaInfos[i];
        CoreField fieldAfterRensa(plan.field());
        for (int x = 1; x <= CoreField::WIDTH; ++x) {
            for (int y = 1; y <= 13; ++y) { // TODO: 13?
                if (rensaInfo.trackResult.erasedAt(x, y) != 0)
                    fieldAfterRensa.unsafeSet(x, y, EMPTY);
            }
            fieldAfterRensa.recalcHeightOn(x);
        }

        CoreField fieldAfterDrop(fieldAfterRensa);
        fieldAfterDrop.forceDrop();

        double s = 0;
        s += evalRensaChainFeature(plan, rensaInfo);
#if USE_HAND_WIDTH_FEATURE
        s += evalRensaHandWidthFeature(plan, rensaInfo);
#endif
#if USE_CONNECTION_FEATURE
        s += evalRensaConnectionFeature(plan, fieldAfterDrop);
#endif
        s += evalRensaGarbageFeature(plan, fieldAfterDrop);

        if (s > maxRensaScore)
            maxRensaScore = s;
    }
    score += maxRensaScore;

    return EvalResult(score, "");
}

double Evaluator::evalFrameFeature(const RefPlan& plan)
{
    double s = 0;
    s += feature_.score(TOTAL_FRAMES, plan.totalFrames());
    // TODO(mayah): Why totalFrames is 0?
    if (plan.totalFrames() != 0)
        s += feature_.score(TOTAL_FRAMES_INVERSE, 1.0 / plan.totalFrames());
    return s;
}

double Evaluator::evalEmptyAvailabilityFeature(const RefPlan& plan)
{
    const CoreField& field = plan.field();

    int emptyCells = 72 - field.countPuyos();
    if (emptyCells <= 0)
        return 0.0;

    EvaluationFeatureKey k[3][3] = {
        { EMPTY_AVAILABILITY_00, EMPTY_AVAILABILITY_01, EMPTY_AVAILABILITY_02, },
        { EMPTY_AVAILABILITY_01, EMPTY_AVAILABILITY_11, EMPTY_AVAILABILITY_12, },
        { EMPTY_AVAILABILITY_02, EMPTY_AVAILABILITY_12, EMPTY_AVAILABILITY_22, },
    };

    double s = 0;
    for (int x = CoreField::WIDTH; x >= 1; --x) {
        for (int y = CoreField::HEIGHT; y >= 1; --y) {
            if (field.color(x, y) != EMPTY)
                continue;

            int left = 2, right = 2;
            if (field.color(x - 1, y) == EMPTY) --left;
            if (field.color(x - 1, y + 1) == EMPTY) --left;
            if (field.color(x + 1, y) == EMPTY) --right;
            if (field.color(x + 1, y + 1) == EMPTY) --right;

            s += feature_.score(k[left][right], 1);
        }
    }

    s += 1.0 / emptyCells;
    return s;
}

template<typename Feature, typename T>
static double calculateConnection(Feature& feature, const CoreField& field, const T keys[])
{
    FieldBitField checked;
    double s = 0;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = 1; field.color(x, y) != EMPTY; ++y) {
            if (!isNormalColor(field.color(x, y)))
                continue;
            if (checked.get(x, y))
                continue;

            int numConnected = field.connectedPuyoNums(x, y, &checked);
            DCHECK(1 <= numConnected && numConnected <= 3);
            s += feature.score(keys[numConnected - 1], 1);
        }
    }

    return s;
}

double Evaluator::evalConnectionFeature(const RefPlan& plan)
{
    static const EvaluationFeatureKey keys[] = {
        CONNECTION_1, CONNECTION_2, CONNECTION_3,
    };

    return calculateConnection(feature_, plan.field(), keys);
}

// Takes 2x3 field, and counts each color puyo number.
template<typename Feature, typename T>
static double calculateDensity(Feature& feature, const CoreField& field, const T keys[])
{
    double s = 0;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = 1; y <= CoreField::HEIGHT + 1; ++y) {
            int numColors[PUYO_COLORS] = { 0 };

            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    numColors[field.color(x + dx, y + dy)] += 1;
                }
            }

            for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
                PuyoColor c = normalPuyoColorOf(i);
                if (numColors[c] > 4)
                    numColors[c] = 4;
                feature.score(keys[numColors[c]], 1);
            }
        }
    }
    return s;
}

double Evaluator::evalDensityFeature(const RefPlan& plan)
{
    static const EvaluationFeatureKey keys[] = {
        DENSITY_0, DENSITY_1, DENSITY_2, DENSITY_3, DENSITY_4,
    };

    return calculateDensity(feature_, plan.field(), keys);
}

double Evaluator::evalPuyoPattern33Feature(const RefPlan& plan)
{
    const CoreField& field = plan.field();

    int s = 0;
    {
        int x = 2;
        int y = 2;
        int patterns[PUYO_COLORS] = { 0 };

        patterns[field.color(x - 1, y - 1)] |= 1 << 0;
        patterns[field.color(x    , y - 1)] |= 1 << 1;
        patterns[field.color(x + 1, y - 1)] |= 1 << 2;

        patterns[field.color(x - 1, y    )] |= 1 << 3;
        patterns[field.color(x    , y    )] |= 1 << 4;
        patterns[field.color(x + 1, y    )] |= 1 << 5;

        patterns[field.color(x - 1, y + 1)] |= 1 << 6;
        patterns[field.color(x    , y + 1)] |= 1 << 7;
        patterns[field.color(x + 1, y + 1)] |= 1 << 8;

        s += feature_.score(PUYO_PATTERN_33, patterns[RED]);
        s += feature_.score(PUYO_PATTERN_33, patterns[GREEN]);
        s += feature_.score(PUYO_PATTERN_33, patterns[YELLOW]);
        s += feature_.score(PUYO_PATTERN_33, patterns[BLUE]);
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

        s += feature_.score(PUYO_PATTERN_33, patterns[RED]);
        s += feature_.score(PUYO_PATTERN_33, patterns[GREEN]);
        s += feature_.score(PUYO_PATTERN_33, patterns[YELLOW]);
        s += feature_.score(PUYO_PATTERN_33, patterns[BLUE]);
    }

    return s;
}

double Evaluator::evalFieldHeightFeature(const RefPlan& plan)
{
    double s = 0;

    const CoreField& field = plan.field();

    double sumHeight = 0;
    for (int x = 1; x < CoreField::WIDTH; ++x)
        sumHeight += field.height(x);
    double averageHeight = sumHeight / 6.0;

    double heightSum = 0.0;
    double heightSquareSum = 0.0;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        double diff = abs(field.height(x) - averageHeight);
        heightSum += diff;
        heightSquareSum += diff * diff;
    }

    s += feature_.score(SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSum);
    s += feature_.score(SQUARE_SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSquareSum);
    return s;
}

double Evaluator::evalThirdColumnHeightFeature(const RefPlan& plan)
{
    return feature_.score(THIRD_COLUMN_HEIGHT, plan.field().height(3));
}

double Evaluator::evalOngoingRensaFeature(const RefPlan& plan, int currentFrameId, const Gazer& gazer)
{
    if (gazer.rensaIsOngoing() && gazer.ongoingRensaInfo().rensaResult.score > scoreForOjama(6)) {
        // TODO: 対応が適当すぎる
        if (gazer.ongoingRensaInfo().rensaResult.score >= scoreForOjama(6) &&
            plan.score() >= gazer.ongoingRensaInfo().rensaResult.score &&
            plan.initiatingFrames() <= gazer.ongoingRensaInfo().finishingRensaFrame) {
            LOG(INFO) << plan.decisionText() << " TAIOU";
            return feature_.score(STRATEGY_TAIOU, 1.0);
        }
    }

    if (!plan.isRensaPlan())
        return 0;

    if (plan.field().isZenkeshi()) {
        return feature_.score(STRATEGY_ZENKESHI, 1);
    }

    int rensaEndingFrameId = currentFrameId + plan.totalFrames();
    int estimatedMaxScore = gazer.estimateMaxScore(rensaEndingFrameId);

    // --- 1.1. 十分でかい場合は打って良い。
    // / TODO: 十分でかいとは？ / とりあえず致死量ということにする
    if (plan.score() >= estimatedMaxScore + scoreForOjama(60)) {
        return feature_.score(STRATEGY_LARGE_ENOUGH, 1);
    }

    // --- 1.2. 対応手なく潰せる
    // TODO: 実装があやしい。
    if (plan.score() >= scoreForOjama(18) && estimatedMaxScore <= scoreForOjama(6)) {
        return feature_.score(STRATEGY_TSUBUSHI, 1);
    }

    // --- 1.3. 飽和したので打つしかなくなった
    // TODO: これは EnemyRensaInfo だけじゃなくて MyRensaInfo も必要なのでは……。
    // TODO: 60 個超えたら打つとかなんか間違ってるだろう。
    if (plan.field().countPuyos() >= 60) {
        return feature_.score(STRATEGY_HOUWA, 1);
    }

    // --- 1.4. 打つと有利になる
    // TODO: そもそも数値化の仕方が分からない。

    // 基本的に先打ちすると負けるので、打たないようにする
    //ostringstream ss;
    //ss << "SAKIUCHI will lose : score = " << plan.score() << " EMEMY score = " << estimatedMaxScore << endl;
    // LOG(INFO) << plan.decisionText() << " " << ss.str();
    double s = 0;
    s += feature_.score(STRATEGY_SCORE, plan.score());
    s += feature_.score(STRATEGY_SAKIUCHI, 1.0);
    return s;
}

double Evaluator::evalRensaChainFeature(const RefPlan& /*plan*/, const TrackedPossibleRensaInfo& info)
{
    int numNecessaryPuyos = TsumoPossibility::necessaryPuyos(0.5, info.necessaryPuyoSet.toPuyoSet());

    double s = 0;
    s += feature_.score(MAX_CHAINS, info.rensaResult.chains);
    s += feature_.score(MAX_RENSA_NECESSARY_PUYOS, numNecessaryPuyos);
    if (numNecessaryPuyos != 0)
        s += feature_.score(MAX_RENSA_NECESSARY_PUYOS_INVERSE, 1.0 / numNecessaryPuyos);
    return s;
}

double Evaluator::evalRensaHandWidthFeature(const RefPlan& plan, const TrackedPossibleRensaInfo& info)
{
    // -----
    int distanceCountResult[5] = { 0, 0, 0, 0, 0 };

    // 1 連鎖の部分を距離 1 とし、距離 4 までを求める。
    // 距離 2, 3, 4 の数を数え、その広がり具合により、手の広さを求めることができる。
    int distance[CoreField::MAP_WIDTH][CoreField::MAP_HEIGHT];
    for (int x = 0; x < CoreField::MAP_WIDTH; ++x) {
        for (int y = 0; y < CoreField::MAP_HEIGHT; ++y) {
            if (info.trackResult.erasedAt(x, y) == 1)
                distance[x][y] = 1;
            else
                distance[x][y] = 0;
        }
    }

    // TODO(mayah): Why O(HWD)? Why not writing O(HW)?
    const CoreField& field = plan.field();
    for (int d = 2; d <= 4; ++d) {
        for (int x = 1; x <= CoreField::WIDTH; ++x) {
            for (int y = 1; y <= CoreField::HEIGHT; ++y) {
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

    double s = 0;
    s += feature_.score(HAND_WIDTH_2, d2);
    s += feature_.score(HAND_WIDTH_3, d3);
    s += feature_.score(HAND_WIDTH_4, d4);
    s += feature_.score(HAND_WIDTH_RATIO_32, r32);
    s += feature_.score(HAND_WIDTH_RATIO_43, r43);
    s += feature_.score(HAND_WIDTH_RATIO_32_SQUARED, r32 * r32);
    s += feature_.score(HAND_WIDTH_RATIO_43_SQUARED, r43 * r43);
    return s;
}

double Evaluator::evalRensaConnectionFeature(const RefPlan& plan, const CoreField& fieldAfterDrop)
{
    UNUSED_VARIABLE(plan);

    static const EvaluationFeatureKey keys[] = {
        CONNECTION_AFTER_VANISH_1, CONNECTION_AFTER_VANISH_2, CONNECTION_AFTER_VANISH_3,
    };

    return calculateConnection(feature_, fieldAfterDrop, keys);
}

double Evaluator::evalRensaGarbageFeature(const RefPlan& plan, const CoreField& fieldAfterDrop)
{
    return feature_.score(NUM_GARBAGE_PUYOS, plan.field().countPuyos() - fieldAfterDrop.countPuyos());
}
