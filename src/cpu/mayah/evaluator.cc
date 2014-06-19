#include "evaluator.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <glog/logging.h>

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

#include "feature_parameter.h"
#include "gazer.h"

const bool USE_CONNECTION_FEATURE = false;
const bool USE_EMPTY_AVAILABILITY_FEATURE = false;
const bool USE_HAND_WIDTH_FEATURE = false;
const bool USE_THIRD_COLUMN_HEIGHT_FEATURE = false;

using namespace std;

template<typename ScoreCollector>
void evalFrameFeature(ScoreCollector* sc, const RefPlan& plan)
{
    sc->addScore(TOTAL_FRAMES, plan.totalFrames());
}

template<typename ScoreCollector>
void evalEmptyAvailabilityFeature(ScoreCollector* sc, const RefPlan& plan)
{
    const CoreField& field = plan.field();

    int emptyCells = 72 - field.countPuyos();
    if (emptyCells <= 0)
        return;

    EvaluationFeatureKey k[3][3] = {
        { EMPTY_AVAILABILITY_00, EMPTY_AVAILABILITY_01, EMPTY_AVAILABILITY_02, },
        { EMPTY_AVAILABILITY_01, EMPTY_AVAILABILITY_11, EMPTY_AVAILABILITY_12, },
        { EMPTY_AVAILABILITY_02, EMPTY_AVAILABILITY_12, EMPTY_AVAILABILITY_22, },
    };

    for (int x = CoreField::WIDTH; x >= 1; --x) {
        for (int y = CoreField::HEIGHT; y >= 1; --y) {
            if (field.color(x, y) != EMPTY)
                continue;

            int left = 2, right = 2;
            if (field.color(x - 1, y) == EMPTY) --left;
            if (field.color(x - 1, y + 1) == EMPTY) --left;
            if (field.color(x + 1, y) == EMPTY) --right;
            if (field.color(x + 1, y + 1) == EMPTY) --right;

            sc->addScore(k[left][right], 1);
        }
    }
}

template<typename ScoreCollector>
static void calculateConnection(ScoreCollector* sc, const CoreField& field, const EvaluationFeatureKey keys[])
{
    FieldBitField checked;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = 1; field.color(x, y) != EMPTY; ++y) {
            if (!isNormalColor(field.color(x, y)))
                continue;
            if (checked.get(x, y))
                continue;

            int numConnected = field.connectedPuyoNums(x, y, &checked);
            DCHECK(1 <= numConnected && numConnected <= 3);
            sc->addScore(keys[numConnected - 1], 1);
        }
    }
}

template<typename ScoreCollector>
void evalConnectionFeature(ScoreCollector* sc, const RefPlan& plan)
{
    static const EvaluationFeatureKey keys[] = {
        CONNECTION_1, CONNECTION_2, CONNECTION_3,
    };

    calculateConnection(sc, plan.field(), keys);
}

// Takes 2x3 field, and counts each color puyo number.
template<typename ScoreCollector>
static void calculateDensity(ScoreCollector* sc, const CoreField& field, const EvaluationFeatureKey keys[])
{
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
                sc->addScore(keys[numColors[c]], 1);
            }
        }
    }

}

template<typename ScoreCollector>
void evalDensityFeature(ScoreCollector* sc, const RefPlan& plan)
{
    static const EvaluationFeatureKey keys[] = {
        DENSITY_0, DENSITY_1, DENSITY_2, DENSITY_3, DENSITY_4,
    };

    calculateDensity(sc, plan.field(), keys);
}

template<typename ScoreCollector>
void evalPuyoPattern33Feature(ScoreCollector* sc, const RefPlan& plan)
{
    const CoreField& field = plan.field();

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

        sc->addScore(PUYO_PATTERN_33, patterns[RED]);
        sc->addScore(PUYO_PATTERN_33, patterns[GREEN]);
        sc->addScore(PUYO_PATTERN_33, patterns[YELLOW]);
        sc->addScore(PUYO_PATTERN_33, patterns[BLUE]);
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

        sc->addScore(PUYO_PATTERN_33, patterns[RED]);
        sc->addScore(PUYO_PATTERN_33, patterns[GREEN]);
        sc->addScore(PUYO_PATTERN_33, patterns[YELLOW]);
        sc->addScore(PUYO_PATTERN_33, patterns[BLUE]);
    }
}

template<typename ScoreCollector>
void evalFieldHeightFeature(ScoreCollector* sc, const RefPlan& plan)
{
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

    sc->addScore(SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSum);
    sc->addScore(SQUARE_SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSquareSum);
}

template<typename ScoreCollector>
void evalThirdColumnHeightFeature(ScoreCollector* sc, const RefPlan& plan)
{
    sc->addScore(THIRD_COLUMN_HEIGHT, plan.field().height(3));
}

template<typename ScoreCollector>
void evalValleyDepth(ScoreCollector* sc, const RefPlan& plan)
{
    for (int x = 1; x <= 6; ++x) {
        int currentHeight = plan.field().height(x);
        int leftHeight = (x == 1) ? 12 : plan.field().height(x - 1);
        int rightHeight = (x == 6) ? 12 : plan.field().height(x + 1);

        int leftDepth = max(leftHeight - currentHeight, 0);
        int rightDepth = max(rightHeight - currentHeight, 0);
        sc->addScore(VALLEY_DEPTH, min(leftDepth, rightDepth));
    }
}

template<typename ScoreCollector>
void evalOngoingRensaFeature(ScoreCollector* sc, const RefPlan& plan, int currentFrameId, const Gazer& gazer)
{
    if (gazer.rensaIsOngoing() && gazer.ongoingRensaInfo().rensaResult.score > scoreForOjama(6)) {
        // TODO: 対応が適当すぎる
        if (gazer.ongoingRensaInfo().rensaResult.score >= scoreForOjama(6) &&
            plan.score() >= gazer.ongoingRensaInfo().rensaResult.score &&
            plan.initiatingFrames() <= gazer.ongoingRensaInfo().finishingRensaFrame) {
            LOG(INFO) << plan.decisionText() << " TAIOU";
            sc->addScore(STRATEGY_TAIOU, 1.0);
            return;
        }
    }

    if (!plan.isRensaPlan())
        return;

    if (plan.field().isZenkeshi()) {
        sc->addScore(STRATEGY_ZENKESHI, 1);
        return;
    }

    int rensaEndingFrameId = currentFrameId + plan.totalFrames();
    int estimatedMaxScore = gazer.estimateMaxScore(rensaEndingFrameId);

    // --- 1.1. 十分でかい場合は打って良い。
    // / TODO: 十分でかいとは？ / とりあえず致死量ということにする
    if (plan.score() >= estimatedMaxScore + scoreForOjama(60)) {
        sc->addScore(STRATEGY_LARGE_ENOUGH, 1);
        return;
    }

    // --- 1.2. 対応手なく潰せる
    // TODO: 実装があやしい。
    if (plan.score() >= scoreForOjama(18) && estimatedMaxScore <= scoreForOjama(6)) {
        sc->addScore(STRATEGY_TSUBUSHI, 1);
        return;
    }

    // --- 1.3. 飽和したので打つしかなくなった
    // TODO: これは EnemyRensaInfo だけじゃなくて MyRensaInfo も必要なのでは……。
    // TODO: 60 個超えたら打つとかなんか間違ってるだろう。
    if (plan.field().countPuyos() >= 60) {
        sc->addScore(STRATEGY_HOUWA, 1);
        return;
    }

    // --- 1.4. 打つと有利になる
    // TODO: そもそも数値化の仕方が分からない。

    // 基本的に先打ちすると負けるので、打たないようにする
    //ostringstream ss;
    //ss << "SAKIUCHI will lose : score = " << plan.score() << " EMEMY score = " << estimatedMaxScore << endl;
    // LOG(INFO) << plan.decisionText() << " " << ss.str();
    sc->addScore(STRATEGY_SCORE, plan.score());
    sc->addScore(STRATEGY_SAKIUCHI, 1.0);
}

template<typename ScoreCollector>
void evalRensaChainFeature(ScoreCollector* sc, const RefPlan& /*plan*/, const TrackedPossibleRensaInfo& info)
{
    int numNecessaryPuyos = TsumoPossibility::necessaryPuyos(0.5, info.necessaryPuyoSet.toPuyoSet());

    sc->addScore(MAX_CHAINS, info.rensaResult.chains);
    sc->addScore(MAX_RENSA_NECESSARY_PUYOS, numNecessaryPuyos);
    if (numNecessaryPuyos != 0)
        sc->addScore(MAX_RENSA_NECESSARY_PUYOS_INVERSE, 1.0 / numNecessaryPuyos);
}

template<typename ScoreCollector>
void evalRensaHandWidthFeature(ScoreCollector* sc, const RefPlan& plan, const TrackedPossibleRensaInfo& info)
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

    sc->addScore(HAND_WIDTH_2, d2);
    sc->addScore(HAND_WIDTH_3, d3);
    sc->addScore(HAND_WIDTH_4, d4);
    sc->addScore(HAND_WIDTH_RATIO_32, r32);
    sc->addScore(HAND_WIDTH_RATIO_43, r43);
    sc->addScore(HAND_WIDTH_RATIO_32_SQUARED, r32 * r32);
    sc->addScore(HAND_WIDTH_RATIO_43_SQUARED, r43 * r43);
}

template<typename ScoreCollector>
void evalRensaConnectionFeature(ScoreCollector* sc, const RefPlan& plan, const CoreField& fieldAfterDrop)
{
    UNUSED_VARIABLE(plan);

    static const EvaluationFeatureKey keys[] = {
        CONNECTION_AFTER_VANISH_1, CONNECTION_AFTER_VANISH_2, CONNECTION_AFTER_VANISH_3,
    };

    calculateConnection(sc, fieldAfterDrop, keys);
}

template<typename ScoreCollector>
void evalRensaGarbageFeature(ScoreCollector* sc, const RefPlan& plan, const CoreField& fieldAfterDrop)
{
    sc->addScore(NUM_GARBAGE_PUYOS, plan.field().countPuyos() - fieldAfterDrop.countPuyos());
}

template<typename ScoreCollector>
void eval(ScoreCollector* sc, const RefPlan& plan, int currentFrameId, const Gazer& gazer)
{
    evalFrameFeature(sc, plan);
    if (USE_EMPTY_AVAILABILITY_FEATURE)
        evalEmptyAvailabilityFeature(sc, plan);
    if (USE_CONNECTION_FEATURE)
        evalConnectionFeature(sc, plan);
    evalDensityFeature(sc, plan);
    evalPuyoPattern33Feature(sc, plan);
    evalFieldHeightFeature(sc, plan);
    if (USE_THIRD_COLUMN_HEIGHT_FEATURE)
        evalThirdColumnHeightFeature(sc, plan);
    evalValleyDepth(sc, plan);
    evalOngoingRensaFeature(sc, plan, currentFrameId, gazer);

    vector<TrackedPossibleRensaInfo> rensaInfos =
        RensaDetector::findPossibleRensasWithTracking(plan.field(), Evaluator::NUM_KEY_PUYOS);
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

        ScoreCollector rensaScoreCollector(sc->featureParameter());
        evalRensaChainFeature(&rensaScoreCollector, plan, rensaInfo);
        evalRensaGarbageFeature(&rensaScoreCollector, plan, fieldAfterDrop);
        if (USE_HAND_WIDTH_FEATURE)
            evalRensaHandWidthFeature(&rensaScoreCollector, plan, rensaInfo);
        if (USE_CONNECTION_FEATURE)
            evalRensaConnectionFeature(&rensaScoreCollector, plan, fieldAfterDrop);

        if (rensaScoreCollector.score() > maxRensaScore) {
            maxRensaScore = rensaScoreCollector.score();
            sc->merge(rensaScoreCollector);
        }
    }
}

// ----------------------------------------------------------------------

class UsualScoreCollector {
public:
    explicit UsualScoreCollector(const FeatureParameter& param) : param_(param) {}

    void addScore(EvaluationFeatureKey key, double v) { score_ += param_.score(key, v); }
    void addScore(EvaluationSparseFeatureKey key, int i) { score_ += param_.score(key, i); }
    void merge(const UsualScoreCollector& sc) { score_ += sc.score(); }

    double score() const { return score_; }
    const FeatureParameter& featureParameter() const { return param_; }

private:
    const FeatureParameter& param_;
    double score_ = 0.0;
};

class LearningScoreCollector {
public:
    LearningScoreCollector(const FeatureParameter& param) : collector_(param) {}

    void addScore(EvaluationFeatureKey key, double v)
    {
        collector_.addScore(key, v);
        collectedFeatures_[key] = v;
    }

    void addScore(EvaluationSparseFeatureKey key, int i)
    {
        collector_.addScore(key, i);
        collectedSparseFeatures_[key].push_back(i);
    }

    void merge(const LearningScoreCollector& sc)
    {
        collector_.merge(sc.collector_);
        for (const auto& entry : collectedFeatures_) {
            collectedFeatures_[entry.first] = entry.second;
        }
        for (const auto& entry : collectedSparseFeatures_) {
            collectedSparseFeatures_[entry.first].insert(
                collectedSparseFeatures_[entry.first].end(),
                entry.second.begin(),
                entry.second.end());
        }
    }

    double score() const { return collector_.score(); }
    const FeatureParameter& featureParameter() const { return collector_.featureParameter(); }

    CollectedFeature toCollectedFeature() const {
        return CollectedFeature {
            score(),
            collectedFeatures_,
            collectedSparseFeatures_
        };
    }

private:
    UsualScoreCollector collector_;
    map<EvaluationFeatureKey, double> collectedFeatures_;
    map<EvaluationSparseFeatureKey, vector<int>> collectedSparseFeatures_;
};

double Evaluator::eval(const RefPlan& plan, int currentFrameId, const Gazer& gazer)
{
    UsualScoreCollector sc(param_);
    ::eval(&sc, plan, currentFrameId, gazer);
    return sc.score();
}

CollectedFeature Evaluator::evalWithCollectingFeature(const RefPlan& plan, int currentFrameId, const Gazer& gazer)
{
    LearningScoreCollector sc(param_);
    ::eval(&sc, plan, currentFrameId, gazer);
    return sc.toCollectedFeature();
}
