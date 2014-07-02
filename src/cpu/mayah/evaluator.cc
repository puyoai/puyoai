#include "evaluator.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
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
#include "util.h"

const bool USE_CONNECTION_FEATURE = true;
const bool USE_CONNECTION_HORIZONTAL_FEATURE = true;
const bool USE_HAND_WIDTH_FEATURE = true;
const bool USE_HEIGHT_DIFF_FEATURE = false;
const bool USE_THIRD_COLUMN_HEIGHT_FEATURE = true;
const bool USE_DENSITY_FEATURE = false;
const bool USE_PATTERN33_FEATURE = false;
const bool USE_PATTERN34_FEATURE = true;
const bool USE_IGNITION_HEIGHT_FEATURE = false;
const bool USE_FIELD_USHAPE_FEATURE = true;

using namespace std;

string CollectedFeature::toString() const
{
    stringstream ss;
    ss << "score = " << score_ << endl;
    for (const auto& entry : collectedFeatures_) {
        ss << ::toString(entry.first) << "=" << to_string(entry.second) << endl;
    }
    for (const auto& entry : collectedSparseFeatures_) {
        ss << ::toString(entry.first) << "=";
        for (int v : entry.second)
            ss << v << ' ';
        ss << endl;
    }

    return ss.str();
}

template<typename ScoreCollector>
void evalFrameFeature(ScoreCollector* sc, const RefPlan& plan)
{
    sc->addScore(TOTAL_FRAMES, plan.totalFrames());
    sc->addScore(NUM_CHIGIRI, plan.numChigiri());
}

template<typename ScoreCollector>
static void calculateConnection(ScoreCollector* sc, const CoreField& field, EvaluationSparseFeatureKey key)
{
    FieldBitField checked;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        // TODO(mayah): Why checking EMPTY? Why not height?
        for (int y = 1; field.color(x, y) != EMPTY; ++y) {
            if (!isNormalColor(field.color(x, y)))
                continue;
            if (checked.get(x, y))
                continue;

            int numConnected = field.countConnectedPuyos(x, y, &checked);
            if (numConnected >= 4)
                numConnected = 3;
            sc->addScore(key, numConnected, 1);
        }
    }
}

template<typename ScoreCollector>
void evalConnectionFeature(ScoreCollector* sc, const RefPlan& plan)
{
    calculateConnection(sc, plan.field(), CONNECTION);
}

template<typename ScoreCollector>
void evalConnectionHorizontalFeature(ScoreCollector* sc, const RefPlan& plan)
{
    const int MAX_HEIGHT = 5; // instead of CoreField::HEIGHT
    const CoreField& f = plan.field();
    for (int y = 1; y <= MAX_HEIGHT; ++y) {
        for (int x = 1; x <= CoreField::WIDTH; ++x) {
            if (!isNormalColor(f.color(x, y)))
                continue;

            int len = 1;
            while (f.color(x, y) == f.color(x + len, y))
                ++len;

            DCHECK(0 <= len && len < 4) << len;
            sc->addScore(CONNECTION_HORIZONTAL, len, 1);
            x += len - 1;
        }
    }
}

// Takes 2x3 field, and counts each color puyo number.
template<typename ScoreCollector>
void evalDensityFeature(ScoreCollector* sc, const RefPlan& plan)
{
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = 1; y <= CoreField::HEIGHT + 1; ++y) {
            int numColors[NUM_PUYO_COLORS] = { 0 };

            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    numColors[plan.field().color(x + dx, y + dy)] += 1;
                }
            }

            for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
                PuyoColor c = normalPuyoColorOf(i);
                sc->addScore(DENSITY, c, 1);
            }
        }
    }
}

template<typename ScoreCollector>
void evalPuyoPattern33Feature(ScoreCollector* sc, const RefPlan& plan)
{
    const CoreField& field = plan.field();

    {
        int x = 2;
        int y = 2;
        int patterns[NUM_PUYO_COLORS] = { 0 };

        patterns[field.color(x - 1, y - 1)] |= 1 << 0;
        patterns[field.color(x    , y - 1)] |= 1 << 1;
        patterns[field.color(x + 1, y - 1)] |= 1 << 2;

        patterns[field.color(x - 1, y    )] |= 1 << 3;
        patterns[field.color(x    , y    )] |= 1 << 4;
        patterns[field.color(x + 1, y    )] |= 1 << 5;

        patterns[field.color(x - 1, y + 1)] |= 1 << 6;
        patterns[field.color(x    , y + 1)] |= 1 << 7;
        patterns[field.color(x + 1, y + 1)] |= 1 << 8;

        sc->addScore(PUYO_PATTERN_33, patterns[RED], 1);
        sc->addScore(PUYO_PATTERN_33, patterns[GREEN], 1);
        sc->addScore(PUYO_PATTERN_33, patterns[YELLOW], 1);
        sc->addScore(PUYO_PATTERN_33, patterns[BLUE], 1);
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

        sc->addScore(PUYO_PATTERN_33, patterns[RED], 1);
        sc->addScore(PUYO_PATTERN_33, patterns[GREEN], 1);
        sc->addScore(PUYO_PATTERN_33, patterns[YELLOW], 1);
        sc->addScore(PUYO_PATTERN_33, patterns[BLUE], 1);
    }
}

template<typename ScoreCollector>
void evalPuyoPattern34Feature(ScoreCollector* sc, const RefPlan& plan)
{
    const CoreField& field = plan.field();

    int patternsLeft[NUM_PUYO_COLORS] = { 0 };
    int patternsRight[NUM_PUYO_COLORS] = { 0 };
    for (int y = 1; y <= 3; ++y) {
        for (int x = 1; x <= 4; ++x) {
            patternsLeft[field.color(x, y)] |= 1 << ((y - 1) * 4 + (x - 1));
            patternsRight[field.color(7 - x, y)] |= 1 << ((y - 1) * 4 + (x - 1));
        }
    }

    sc->addScore(PUYO_PATTERN_34_LEFT, patternsLeft[RED], 1);
    sc->addScore(PUYO_PATTERN_34_LEFT, patternsLeft[GREEN], 1);
    sc->addScore(PUYO_PATTERN_34_LEFT, patternsLeft[YELLOW], 1);
    sc->addScore(PUYO_PATTERN_34_LEFT, patternsLeft[BLUE], 1);

    sc->addScore(PUYO_PATTERN_34_RIGHT, patternsRight[RED], 1);
    sc->addScore(PUYO_PATTERN_34_RIGHT, patternsRight[GREEN], 1);
    sc->addScore(PUYO_PATTERN_34_RIGHT, patternsRight[YELLOW], 1);
    sc->addScore(PUYO_PATTERN_34_RIGHT, patternsRight[BLUE], 1);
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
    sc->addScore(THIRD_COLUMN_HEIGHT, plan.field().height(3), 1);
}

template<typename ScoreCollector>
void evalValleyDepthRidgeHeight(ScoreCollector* sc, const RefPlan& plan)
{
    for (int x = 1; x <= 6; ++x) {
        int currentHeight = plan.field().height(x);
        int leftHeight = (x == 1) ? 14 : plan.field().height(x - 1);
        int rightHeight = (x == 6) ? 14 : plan.field().height(x + 1);

        // --- valley
        {
            int left = max(leftHeight - currentHeight, 0);
            int right = max(rightHeight - currentHeight, 0);
            int depth = min(left, right);
            DCHECK(0 <= depth && depth <= 14) << depth;
            sc->addScore(VALLEY_DEPTH, depth, 1);
        }

        // --- ridge
        {
            int left = max(currentHeight - leftHeight, 0);
            int right = max(currentHeight - rightHeight, 0);
            int height = min(left, right);
            DCHECK(0 <= height && height <= 14) << height;
            sc->addScore(RIDGE_HEIGHT, height, 1);
        }
    }
}

template<typename ScoreCollector>
void evalFieldUShape(ScoreCollector* sc, const RefPlan& plan)
{
    const CoreField& f = plan.field();
    int sumHeight = 0;
    for (int x = 1; x <= 6; ++x)
        sumHeight += f.height(x);

    int s = 0;
    s += abs((f.height(1) - 3) * 6 - sumHeight);
    s += abs((f.height(2) - 0) * 6 - sumHeight);
    s += abs((f.height(3) + 1) * 6 - sumHeight);
    s += abs((f.height(4) + 1) * 6 - sumHeight);
    s += abs((f.height(5) - 0) * 6 - sumHeight);
    s += abs((f.height(6) - 3) * 6 - sumHeight);

    sc->addScore(FIELD_USHAPE, s);
}

template<typename ScoreCollector>
void evalUnreachableSpace(ScoreCollector* sc, const RefPlan& plan)
{
    const CoreField& f = plan.field();
    FieldBitField checked;
    f.countConnectedPuyos(3, 12, &checked);

    int countUnreachable = 0;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = f.height(x) + 1; y <= CoreField::HEIGHT; ++y) {
            if (f.color(x, y) != EMPTY)
                continue;
            if (checked.get(x, y))
                continue;
            countUnreachable++;
        }
    }

    sc->addScore(NUM_UNREACHABLE_SPACE, countUnreachable);
}

// Returns true If we don't need to evaluate other features.
template<typename ScoreCollector>
bool evalStrategy(ScoreCollector* sc, const RefPlan& plan, const CoreField& currentField,
                  int currentFrameId, const Gazer& gazer)
{
    if (!plan.isRensaPlan())
        return false;

    if (gazer.rensaIsOngoing() && gazer.ongoingRensaInfo().rensaResult.score > scoreForOjama(6)) {
        // TODO: 対応が適当すぎる
        if (gazer.ongoingRensaInfo().rensaResult.score >= scoreForOjama(6) &&
            plan.score() >= gazer.ongoingRensaInfo().rensaResult.score &&
            plan.initiatingFrames() <= gazer.ongoingRensaInfo().finishingRensaFrame) {
            LOG(INFO) << plan.decisionText() << " TAIOU";

            sc->addScore(STRATEGY_TAIOU, 1.0);
            return false;
        }
    }

    sc->addScore(STRATEGY_SCORE, plan.score());

    if (plan.field().isZenkeshi()) {
        sc->addScore(STRATEGY_ZENKESHI, 1);
        return true;
    }

    int rensaEndingFrameId = currentFrameId + plan.totalFrames();
    int estimatedMaxScore = gazer.estimateMaxScore(rensaEndingFrameId);

    // --- If the rensa is large enough, fire it.
    if (plan.score() >= estimatedMaxScore + scoreForOjama(60)) {
        sc->addScore(STRATEGY_LARGE_ENOUGH, 1);
        return true;
    }

    // --- If we can send 18>= ojamas, and opponent does not have any hand to cope with it,
    // we can fire it.
    // TODO(mayah): We need to check if the enemy cannot fire his rensa after ojama is dropped.
    if (plan.score() >= scoreForOjama(18) && estimatedMaxScore <= scoreForOjama(6)) {
        sc->addScore(STRATEGY_TSUBUSHI, 1);
        return true;
    }

    if (currentField.countPuyos() >= 60) {
        sc->addScore(STRATEGY_HOUWA, 1);
        return false;
    }

    sc->addScore(STRATEGY_SAKIUCHI, 1.0);
    return false;
}

template<typename ScoreCollector>
void evalRensaStrategy(ScoreCollector* sc, const RefPlan& plan,
                       const TrackedPossibleRensaInfo& info,
                       int currentFrameId, const Gazer& gazer)
{
    UNUSED_VARIABLE(info);
    UNUSED_VARIABLE(currentFrameId);

#if 0
    int rensaEndingFrameId = currentFrameId + plan.totalFrames();
    int estimatedEnemyMaxScore = gazer.estimateMaxScore(rensaEndingFrameId);
#endif

    // TODO(mayah): Ah, maybe sakiuchi etc. wins this value?
    if (plan.score() >= scoreForOjama(15) && plan.score() <= scoreForOjama(30) && plan.field().countPuyos() >= 40) {
        if (!gazer.rensaIsOngoing())
            sc->addScore(STRATEGY_SAISOKU, 1);
    }
}

template<typename ScoreCollector>
void evalRensaChainFeature(ScoreCollector* sc, const RefPlan& /*plan*/, const TrackedPossibleRensaInfo& info)
{
    int numNecessaryPuyos = TsumoPossibility::necessaryPuyos(0.5, info.necessaryPuyoSet.toPuyoSet());

    sc->addScore(MAX_CHAINS, info.rensaResult.chains, 1);
    sc->addScore(MAX_RENSA_NECESSARY_PUYOS, numNecessaryPuyos);
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

    sc->addScore(HAND_WIDTH_2, distanceCountResult[2] > 10 ? 10 : distanceCountResult[2], 1);
    sc->addScore(HAND_WIDTH_3, distanceCountResult[3] > 10 ? 10 : distanceCountResult[3], 1);
    sc->addScore(HAND_WIDTH_4, distanceCountResult[4] > 10 ? 10 : distanceCountResult[4], 1);
}

template<typename ScoreCollector>
void evalRensaIgnitionHeightFeature(ScoreCollector* sc, const RefPlan& plan, const TrackedPossibleRensaInfo& info)
{
    for (int y = CoreField::HEIGHT; y >= 1; --y) {
        for (int x = 1; x <= CoreField::WIDTH; ++x) {
            if (!isNormalColor(plan.field().color(x, y)))
                continue;
            if (info.trackResult.erasedAt(x, y) == 1) {
                sc->addScore(IGNITION_HEIGHT, y, 1);
                return;
            }
        }
    }

    sc->addScore(IGNITION_HEIGHT, 0, 1);
}

template<typename ScoreCollector>
void evalRensaConnectionFeature(ScoreCollector* sc, const RefPlan& plan, const CoreField& fieldAfterDrop)
{
    UNUSED_VARIABLE(plan);
    calculateConnection(sc, fieldAfterDrop, CONNECTION_AFTER_VANISH);
}

template<typename ScoreCollector>
void evalRensaGarbageFeature(ScoreCollector* sc, const RefPlan& plan, const CoreField& fieldAfterDrop)
{
    sc->addScore(NUM_GARBAGE_PUYOS, plan.field().countPuyos() - fieldAfterDrop.countPuyos());
}

template<typename ScoreCollector>
void evalCountPuyoFeature(ScoreCollector* sc, const RefPlan& plan)
{
    sc->addScore(NUM_COUNT_PUYOS, plan.field().countColorPuyos(), 1);
}

template<typename ScoreCollector>
void eval(ScoreCollector* sc, const RefPlan& plan, const CoreField& currentField, int currentFrameId, bool fast, const Gazer& gazer)
{
    if (evalStrategy(sc, plan, currentField, currentFrameId, gazer))
        return;

    evalFrameFeature(sc, plan);
    evalCountPuyoFeature(sc, plan);
    if (USE_CONNECTION_FEATURE)
        evalConnectionFeature(sc, plan);
    if (USE_CONNECTION_HORIZONTAL_FEATURE)
        evalConnectionHorizontalFeature(sc, plan);
    if (USE_DENSITY_FEATURE)
        evalDensityFeature(sc, plan);
    if (USE_PATTERN33_FEATURE)
        evalPuyoPattern33Feature(sc, plan);
    if (USE_PATTERN34_FEATURE)
        evalPuyoPattern34Feature(sc, plan);
    if (USE_HEIGHT_DIFF_FEATURE)
        evalFieldHeightFeature(sc, plan);
    if (USE_THIRD_COLUMN_HEIGHT_FEATURE)
        evalThirdColumnHeightFeature(sc, plan);
    evalValleyDepthRidgeHeight(sc, plan);
    if (USE_FIELD_USHAPE_FEATURE)
        evalFieldUShape(sc, plan);
    evalUnreachableSpace(sc, plan);

    int keyPuyos = fast ? 0 : Evaluator::NUM_KEY_PUYOS;

    vector<TrackedPossibleRensaInfo> rensaInfos =
        RensaDetector::findPossibleRensasWithTracking(plan.field(), keyPuyos);

    double maxRensaScore = 0;
    unique_ptr<ScoreCollector> maxRensaScoreCollector;
    for (size_t i = 0; i < rensaInfos.size(); ++i) {
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

        unique_ptr<ScoreCollector> rensaScoreCollector(new ScoreCollector(sc->featureParameter()));
        evalRensaChainFeature(rensaScoreCollector.get(), plan, rensaInfo);
        evalRensaGarbageFeature(rensaScoreCollector.get(), plan, fieldAfterDrop);
        if (USE_HAND_WIDTH_FEATURE)
            evalRensaHandWidthFeature(rensaScoreCollector.get(), plan, rensaInfo);
        if (USE_IGNITION_HEIGHT_FEATURE)
            evalRensaIgnitionHeightFeature(rensaScoreCollector.get(), plan, rensaInfo);
        if (USE_CONNECTION_FEATURE)
            evalRensaConnectionFeature(rensaScoreCollector.get(), plan, fieldAfterDrop);
        evalRensaStrategy(rensaScoreCollector.get(), plan, rensaInfo, currentFrameId, gazer);
        if (rensaScoreCollector->score() > maxRensaScore) {
            maxRensaScore = rensaScoreCollector->score();
            maxRensaScoreCollector = move(rensaScoreCollector);
        }
    }

    if (maxRensaScoreCollector.get())
        sc->merge(*maxRensaScoreCollector);
}

// ----------------------------------------------------------------------

class UsualScoreCollector {
public:
    explicit UsualScoreCollector(const FeatureParameter& param) : param_(param) {}

    void addScore(EvaluationFeatureKey key, double v) { score_ += param_.score(key, v); }
    void addScore(EvaluationSparseFeatureKey key, int idx, int n) { score_ += param_.score(key, idx, n); }
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

    void addScore(EvaluationSparseFeatureKey key, int idx, int n)
    {
        collector_.addScore(key, idx, n);
        for (int i = 0; i < n; ++i)
            collectedSparseFeatures_[key].push_back(idx);
    }

    void merge(const LearningScoreCollector& sc)
    {
        collector_.merge(sc.collector_);

        for (const auto& entry : sc.collectedFeatures_) {
            collectedFeatures_[entry.first] = entry.second;
        }
        for (const auto& entry : sc.collectedSparseFeatures_) {
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

double Evaluator::eval(const RefPlan& plan, const CoreField& currentField, int currentFrameId, bool fast, const Gazer& gazer)
{
    UsualScoreCollector sc(param_);
    ::eval(&sc, plan, currentField, currentFrameId, fast, gazer);
    return sc.score();
}

CollectedFeature Evaluator::evalWithCollectingFeature(const RefPlan& plan, const CoreField& currentField,
                                                      int currentFrameId, bool fast, const Gazer& gazer)
{
    LearningScoreCollector sc(param_);
    ::eval(&sc, plan, currentField, currentFrameId, fast, gazer);
    return sc.toCollectedFeature();
}
