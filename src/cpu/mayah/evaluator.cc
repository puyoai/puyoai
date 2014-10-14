#include "evaluator.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>

#include <glog/logging.h>

#include "base/time.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/algorithm/rensa_info.h"
#include "core/algorithm/rensa_detector.h"
#include "core/constant.h"
#include "core/decision.h"
#include "core/position.h"
#include "core/field/core_field.h"
#include "core/field/field_bit_field.h"
#include "core/field/rensa_result.h"
#include "core/score.h"

#include "book_field.h"
#include "feature_parameter.h"
#include "gazer.h"

using namespace std;

namespace {

const bool USE_CONNECTION_FEATURE = true;
const bool USE_CONNECTION_HORIZONTAL_FEATURE = false;
const bool USE_RESTRICTED_CONNECTION_HORIZONTAL_FEATURE = true;
const bool USE_HAND_WIDTH_FEATURE = true;
const bool USE_HEIGHT_DIFF_FEATURE = false;
const bool USE_THIRD_COLUMN_HEIGHT_FEATURE = true;
const bool USE_DENSITY_FEATURE = false;
const bool USE_IGNITION_HEIGHT_FEATURE = true;
const bool USE_FIELD_USHAPE_FEATURE = true;
const bool USE_RIDGE_FEATURE = true;
const bool USE_VALLEY_FEATURE = true;
const bool USE_FIRE_POINT_TABOO_FEATURE = true;
const bool USHAPE_ABS = false;
const bool USHAPE_SQUARE = true;

}

template<typename ScoreCollector>
static void calculateConnection(ScoreCollector* sc, const CoreField& field, EvaluationSparseFeatureKey key)
{
    FieldBitField checked;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        // TODO(mayah): Why checking EMPTY? Why not height?
        int height = field.height(x);
        for (int y = 1; y <= height; ++y) {
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

PreEvalResult PreEvaluator::preEval(const CoreField& currentField)
{
    vector<bool> booksMatchable(books_.size());
    for (size_t i = 0; i < books_.size(); ++i) {
        booksMatchable[i] = books_[i].match(currentField).score > 0;
    }

    return PreEvalResult(booksMatchable);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalBook(const std::vector<BookField>& books,
                                         const std::vector<bool>& bookMatchable,
                                         const RefPlan& plan)
{
    double maxScore = 0;
    const BookField* bestBf = nullptr;

    int totalPuyoCount = plan.field().countPuyos();
    if (totalPuyoCount == 0)
        return;

    for (size_t i = 0; i < books.size(); ++i) {
        if (!bookMatchable[i])
            continue;

        const auto& bf = books[i];
        BookField::MatchResult mr = bf.match(plan.field());
        double ratio = mr.count / static_cast<double>(totalPuyoCount);
        DCHECK(0 <= ratio && ratio <= 1.0) << ratio;
        // TODO(mayah): Make this configurable?
        if (ratio < 0.5)
            continue;
        ratio = (ratio - 0.5) * 2;
        double score = mr.score * ratio / totalPuyoCount;
        if (maxScore < score) {
            bestBf = &bf;
            maxScore = score;
        }
    }

    if (bestBf) {
        sc_->addScore(BOOK, maxScore);
        sc_->setBookName(bestBf->name());
    }
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalFrameFeature(const RefPlan& plan)
{
    sc_->addScore(TOTAL_FRAMES, plan.totalFrames());
    sc_->addScore(NUM_CHIGIRI, plan.numChigiri());
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::collectScoreForConnection(const CoreField& field)
{
    calculateConnection(sc_, field, CONNECTION);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalConnectionHorizontalFeature(const RefPlan& plan)
{
    const int MAX_HEIGHT = 3; // instead of CoreField::HEIGHT
    const CoreField& f = plan.field();
    for (int y = 1; y <= MAX_HEIGHT; ++y) {
        for (int x = 1; x <= CoreField::WIDTH; ++x) {
            if (!isNormalColor(f.color(x, y)))
                continue;

            int len = 1;
            while (f.color(x, y) == f.color(x + len, y))
                ++len;

            DCHECK(0 <= len && len < 4) << len;
            sc_->addScore(CONNECTION_HORIZONTAL, len, 1);
            x += len - 1;
        }
    }
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalRestrictedConnectionHorizontalFeature(const RefPlan& plan)
{
    static const EvaluationSparseFeatureKey keys[] = {
        SPARSE_INVALID,
        CONNECTION_HORIZONTAL_FROM_1,
        CONNECTION_HORIZONTAL_FROM_2,
        CONNECTION_HORIZONTAL_FROM_3,
        CONNECTION_HORIZONTAL_FROM_4,
        CONNECTION_HORIZONTAL_FROM_5,
    };

    const int MAX_HEIGHT = 3; // instead of CoreField::HEIGHT
    const CoreField& f = plan.field();
    for (int y = 1; y <= MAX_HEIGHT; ++y) {
        for (int x = 1; x < CoreField::WIDTH; ++x) {
            if (!isNormalColor(f.color(x, y)))
                continue;

            int len = 1;
            while (f.color(x, y) == f.color(x + len, y))
                ++len;

            sc_->addScore(keys[x], len, 1);
            x += len - 1;
        }
    }
}

// Takes 2x3 field, and counts each color puyo number.
template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalDensityFeature(const RefPlan& plan)
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
                PuyoColor c = NORMAL_PUYO_COLORS[i];
                sc_->addScore(DENSITY, c, 1);
            }
        }
    }
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalFieldHeightFeature(const RefPlan& plan)
{
    const CoreField& field = plan.field();

    double sumHeight = 0;
    for (int x = 1; x < CoreField::WIDTH; ++x)
        sumHeight += field.height(x);
    double averageHeight = sumHeight / 6.0;

    double heightSum = 0.0;
    double heightSquareSum = 0.0;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        double diff = std::abs(field.height(x) - averageHeight);
        heightSum += diff;
        heightSquareSum += diff * diff;
    }

    sc_->addScore(SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSum);
    sc_->addScore(SQUARE_SUM_OF_HEIGHT_DIFF_FROM_AVERAGE, heightSquareSum);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalThirdColumnHeightFeature(const RefPlan& plan)
{
    sc_->addScore(THIRD_COLUMN_HEIGHT, plan.field().height(3), 1);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalValleyDepth(const RefPlan& plan)
{
    for (int x = 1; x <= 6; ++x) {
        int currentHeight = plan.field().height(x);
        int leftHeight = (x == 1) ? 14 : plan.field().height(x - 1);
        int rightHeight = (x == 6) ? 14 : plan.field().height(x + 1);

        int left = std::max(leftHeight - currentHeight, 0);
        int right = std::max(rightHeight - currentHeight, 0);
        int depth = std::min(left, right);
        DCHECK(0 <= depth && depth <= 14) << depth;
        sc_->addScore(VALLEY_DEPTH, depth, 1);
    }
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalRidgeHeight(const RefPlan& plan)
{
    for (int x = 1; x <= 6; ++x) {
        int currentHeight = plan.field().height(x);
        int leftHeight = (x == 1) ? 14 : plan.field().height(x - 1);
        int rightHeight = (x == 6) ? 14 : plan.field().height(x + 1);

        int left = std::max(currentHeight - leftHeight, 0);
        int right = std::max(currentHeight - rightHeight, 0);
        int height = std::min(left, right);
        DCHECK(0 <= height && height <= 14) << height;
        sc_->addScore(RIDGE_HEIGHT, height, 1);
    }
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalFieldUShape(const RefPlan& plan, bool enemyHasZenkeshi)
{
    static const int DIFF[CoreField::MAP_WIDTH] = {
        0, -3, 0, 1, 1, 0, -3, 0,
    };

    static const int DIFF_ON_ZENKESHI[FieldConstant::MAP_WIDTH] = {
        0, 2, 2, 2, -8, -6, -6, 0
    };

    const int* diff = enemyHasZenkeshi ? DIFF_ON_ZENKESHI : DIFF;

    const CoreField& f = plan.field();
    double average = 0;
    for (int x = 1; x <= 6; ++x)
        average += (f.height(x) + diff[x]);
    average /= 6;

    double s = 0;
    if (enemyHasZenkeshi) {
        for (int x = 1; x <= CoreField::WIDTH; ++x) {
            int h = f.height(x) + diff[x];
            s += std::abs(h - average);
        }
    } else if (USHAPE_ABS) {
        for (int x = 1; x <= CoreField::WIDTH; ++x) {
            int h = f.height(x) + diff[x];
            if (f.height(x) <= 4) {
                s += 0.01 * std::abs(h - average);
            } else {
                s += std::abs(h - average);
            }
        }
    } else if (USHAPE_SQUARE) {
        for (int x = 1; x <= CoreField::WIDTH; ++x) {
            int h = f.height(x) + diff[x];
            if (f.height(x) <= 4) {
                s += 0.01 * (h - average) * (h - average);
            } else {
                s += (h - average) * (h - average);
            }
        }
    }

    auto key = enemyHasZenkeshi ? FIELD_USHAPE_ON_ZENKESHI : FIELD_USHAPE;
    sc_->addScore(key, s);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalUnreachableSpace(const RefPlan& plan)
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

    sc_->addScore(NUM_UNREACHABLE_SPACE, countUnreachable);
}

// Returns true If we don't need to evaluate other features.
template<typename ScoreCollector>
bool Evaluator<ScoreCollector>::evalStrategy(const RefPlan& plan, const CoreField& currentField,
                                             int currentFrameId, const GazeResult& gazeResult)
{
    if (!plan.isRensaPlan())
        return false;

    if (gazeResult.isRensaOngoing() && gazeResult.ongoingRensaResult().score > scoreForOjama(6)) {
        if ((plan.score() >= gazeResult.ongoingRensaResult().score) &&
            (currentFrameId + plan.framesToInitiate() < gazeResult.ongoingRensaFinishingFrameId())) {
            LOG(INFO) << plan.decisionText() << " TAIOU";
            sc_->addScore(STRATEGY_TAIOU, 1.0);
            return false;
        }
    }

    sc_->addScore(STRATEGY_SCORE, plan.score());

    if (plan.field().isZenkeshi()) {
        int puyoCount = plan.decisions().size() * 2 + currentField.countPuyos();
        if (puyoCount <= 16) {
            sc_->addScore(STRATEGY_INITIAL_ZENKESHI, 1);
            return true;
        }
        sc_->addScore(STRATEGY_ZENKESHI, 1);
        return true;
    }

    if (gazeResult.additionalThoughtInfo().hasZenkeshi() && !gazeResult.additionalThoughtInfo().enemyHasZenkeshi()) {
        if (!gazeResult.isRensaOngoing()) {
            sc_->addScore(STRATEGY_ZENKESHI_CONSUME, 1);
            return false;
        }
        if (gazeResult.isRensaOngoing() && gazeResult.ongoingRensaResult().score <= scoreForOjama(36)) {
            sc_->addScore(STRATEGY_ZENKESHI_CONSUME, 1);
            return false;
        }
    }

    int rensaEndingFrameId = currentFrameId + plan.totalFrames();
    int estimatedMaxScore = gazeResult.estimateMaxScore(rensaEndingFrameId);

    // --- If the rensa is large enough, fire it.
    if (plan.score() >= estimatedMaxScore + scoreForOjama(60)) {
        sc_->addScore(STRATEGY_LARGE_ENOUGH, 1);
        return true;
    }

    // If the rensa score is larger than 80,000, usually it's really large enough.
    if (plan.score() >= 80000) {
        sc_->addScore(STRATEGY_LARGE_ENOUGH, 1);
        return true;
    }

    // --- If we can send 18>= ojamas, and opponent does not have any hand to cope with it,
    // we can fire it.
    // TODO(mayah): We need to check if the enemy cannot fire his rensa after ojama is dropped.
    if (plan.score() >= scoreForOjama(18) && estimatedMaxScore <= scoreForOjama(6)) {
        sc_->addScore(STRATEGY_TSUBUSHI, 1);
        return true;
    }

    sc_->addScore(STRATEGY_SAKIUCHI, 1.0);

    // TODO(mayah): Check land leveling.

    return false;
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaStrategy(const RefPlan& plan, const RensaResult& rensaResult,
                                                       const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                                                       int currentFrameId, const GazeResult& gazeResult)
{
    UNUSED_VARIABLE(currentFrameId);

    // TODO(mayah): Ah, maybe sakiuchi etc. wins this value?
    if (plan.field().countPuyos() >= 36 && plan.score() >= scoreForOjama(15) && plan.chains() <= 3 && rensaResult.chains >= 7 &&
        keyPuyos.size() + firePuyos.size() <= 3 && !gazeResult.isRensaOngoing()) {
        sc_->addScore(STRATEGY_SAISOKU, 1);
    }
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaChainFeature(const RefPlan& plan,
                                                           const RensaResult& rensaResult,
                                                           const ColumnPuyoList& keyPuyos,
                                                           const ColumnPuyoList& firePuyos)
{
    int numKeyPuyos = std::min(7, static_cast<int>(keyPuyos.size()));
    int numFirePuyos = std::min(7, static_cast<int>(firePuyos.size()));

    sc_->addScore(MAX_CHAINS, rensaResult.chains, 1);
    if (plan.field().countPuyos() <= 24) {
        sc_->addScore(MAX_RENSA_KEY_PUYOS_EARLY, numKeyPuyos, 1);
        sc_->addScore(MAX_RENSA_FIRE_PUYOS_EARLY, numFirePuyos, 1);
    } else if (plan.field().countPuyos() <= 42) {
        sc_->addScore(MAX_RENSA_KEY_PUYOS_MIDDLE, numKeyPuyos, 1);
        sc_->addScore(MAX_RENSA_FIRE_PUYOS_MIDDLE, numFirePuyos, 1);
    } else {
        sc_->addScore(MAX_RENSA_KEY_PUYOS_LATE, numKeyPuyos, 1);
        sc_->addScore(MAX_RENSA_FIRE_PUYOS_LATE, numFirePuyos, 1);
    }
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaHandWidthFeature(const RefPlan& plan, const RensaTrackResult& trackResult)
{
    const CoreField& field = plan.field();

    int distanceCount[5] {};
    int distance[CoreField::MAP_WIDTH][CoreField::MAP_HEIGHT] {};

    // TODO(mayah): Using std::queue is 2x slower here.
    Position q[CoreField::MAP_WIDTH * CoreField::MAP_HEIGHT];
    Position* qHead = q;
    Position* qTail = q;

    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = 1; y <= CoreField::HEIGHT; ++y) {
            if (trackResult.erasedAt(x, y) == 1) {
                distanceCount[1]++;
                distance[x][y] = 1;
                *qTail++ = Position(x, y);
            }
        }
    }

    while (qHead != qTail) {
        int x = qHead->x;
        int y = qHead->y;
        qHead++;

        if (distance[x][y] > 3)
            continue;

        int d = distance[x][y] + 1;

        if (distance[x + 1][y] == 0 && field.color(x + 1, y) == PuyoColor::EMPTY) {
            distance[x + 1][y] = d;
            distanceCount[d]++;
            *qTail++ = Position(x + 1, y);
        }
        if (distance[x - 1][y] == 0 && field.color(x - 1, y) == PuyoColor::EMPTY) {
            distance[x - 1][y] = d;
            distanceCount[d]++;
            *qTail++ = Position(x - 1, y);
        }
        if (distance[x][y + 1] == 0 && field.color(x, y + 1) == PuyoColor::EMPTY) {
            distance[x][y + 1] = d;
            distanceCount[d]++;
            *qTail++ = Position(x, y + 1);
        }
        if (distance[x][y - 1] == 0 && field.color(x, y - 1) == PuyoColor::EMPTY) {
            distance[x][y - 1] = d;
            distanceCount[d]++;
            *qTail++ = Position(x, y - 1);
        }
    }

    sc_->addScore(HAND_WIDTH_2, distanceCount[2] > 10 ? 10 : distanceCount[2], 1);
    sc_->addScore(HAND_WIDTH_3, distanceCount[3] > 10 ? 10 : distanceCount[3], 1);
    sc_->addScore(HAND_WIDTH_4, distanceCount[4] > 10 ? 10 : distanceCount[4], 1);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalFirePointTabooFeature(const RefPlan& plan, const RensaTrackResult& trackResult)
{
    const CoreField& field = plan.field();

    // A_A is taboo generally. Allow this from x == 1 or x == 4.
    for (int x = 2; x <= 3; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (trackResult.erasedAt(x, y) != 1 || trackResult.erasedAt(x + 1, y) != 1 || trackResult.erasedAt(x + 2, y) != 1)
                continue;

            if (isNormalColor(field.color(x, y)) && field.color(x, y) == field.color(x + 2, y) && field.color(x + 1, y) == PuyoColor::EMPTY) {
                sc_->addScore(FIRE_POINT_TABOO, 1);
            }
        }
    }
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaIgnitionHeightFeature(const RefPlan& plan, const RensaTrackResult& trackResult, bool enemyHasZenkeshi)
{
    auto key = enemyHasZenkeshi ? IGNITION_HEIGHT_ON_ENEMY_ZENKESHI : IGNITION_HEIGHT;

    for (int y = CoreField::HEIGHT; y >= 1; --y) {
        for (int x = 1; x <= CoreField::WIDTH; ++x) {
            if (!isNormalColor(plan.field().color(x, y)))
                continue;
            if (trackResult.erasedAt(x, y) == 1) {
                sc_->addScore(key, y, 1);
                return;
            }
        }
    }

    sc_->addScore(key, 0, 1);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaConnectionFeature(const CoreField& fieldAfterDrop)
{
    calculateConnection(sc_, fieldAfterDrop, CONNECTION_AFTER_VANISH);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::collectScoreForRensaGarbage(const CoreField& fieldAfterDrop)
{
    sc_->addScore(NUM_GARBAGE_PUYOS, fieldAfterDrop.countPuyos());
    sc_->addScore(NUM_SIDE_GARBAGE_PUYOS, fieldAfterDrop.height(1) + fieldAfterDrop.height(6));
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalCountPuyoFeature(const RefPlan& plan)
{
    sc_->addScore(NUM_COUNT_PUYOS, plan.field().countColorPuyos(), 1);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::collectScore(const RefPlan& plan, const CoreField& currentField,
                                             int currentFrameId, int maxIteration,
                                             const PreEvalResult& preEvalResult, const GazeResult& gazeResult)
{
    // We'd like to evaluate frame feature always.
    evalFrameFeature(plan);

    if (evalStrategy(plan, currentField, currentFrameId, gazeResult))
        return;

    if (!gazeResult.additionalThoughtInfo().enemyHasZenkeshi())
        evalBook(books_, preEvalResult.booksMatchable(), plan);
    evalCountPuyoFeature(plan);
    if (USE_CONNECTION_FEATURE)
        collectScoreForConnection(plan.field());
    if (USE_CONNECTION_HORIZONTAL_FEATURE)
        evalConnectionHorizontalFeature(plan);
    if (USE_RESTRICTED_CONNECTION_HORIZONTAL_FEATURE)
        evalRestrictedConnectionHorizontalFeature(plan);
    if (USE_DENSITY_FEATURE)
        evalDensityFeature(plan);
    if (USE_HEIGHT_DIFF_FEATURE)
        evalFieldHeightFeature(plan);
    if (USE_THIRD_COLUMN_HEIGHT_FEATURE)
        evalThirdColumnHeightFeature(plan);
    if (USE_VALLEY_FEATURE)
        evalValleyDepth(plan);
    if (USE_RIDGE_FEATURE)
        evalRidgeHeight(plan);
    if (USE_FIELD_USHAPE_FEATURE)
        evalFieldUShape(plan, gazeResult.additionalThoughtInfo().enemyHasZenkeshi());

    evalUnreachableSpace(plan);

    int numPuyo = currentField.countPuyos();
    int maxVirtualRensaResultScore = 0;
    double maxRensaScore = -100000000; // TODO(mayah): Should be negative infty?
    std::unique_ptr<ScoreCollector> maxRensaScoreCollector;
    auto callback = [&](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                        const RensaTrackResult& trackResult, const RensaRefSequence&) {
        std::unique_ptr<ScoreCollector> rensaScoreCollector(new ScoreCollector(sc_->featureParameter()));
        RensaEvaluator<ScoreCollector> rensaEvaluator(books_, rensaScoreCollector.get());

        rensaEvaluator.evalRensaChainFeature(plan, rensaResult, keyPuyos, firePuyos);
        rensaEvaluator.collectScoreForRensaGarbage(fieldAfterRensa);
        if (USE_HAND_WIDTH_FEATURE)
            rensaEvaluator.evalRensaHandWidthFeature(plan, trackResult);
        if (USE_FIRE_POINT_TABOO_FEATURE)
            rensaEvaluator.evalFirePointTabooFeature(plan, trackResult);
        if (USE_IGNITION_HEIGHT_FEATURE)
            rensaEvaluator.evalRensaIgnitionHeightFeature(plan, trackResult, gazeResult.additionalThoughtInfo().enemyHasZenkeshi());
        if (USE_CONNECTION_FEATURE)
            rensaEvaluator.evalRensaConnectionFeature(fieldAfterRensa);

        rensaEvaluator.evalRensaStrategy(plan, rensaResult, keyPuyos, firePuyos, currentFrameId, gazeResult);

        if (rensaScoreCollector->score() > maxRensaScore) {
            maxRensaScore = rensaScoreCollector->score();
            maxRensaScoreCollector = move(rensaScoreCollector);
        }

        double rensaScore = rensaResult.score;
        if (numPuyo > 63)
            rensaScore *= 0.5;
        else if (numPuyo > 61)
            rensaScore *= 0.75;
        else if (numPuyo > 55)
            rensaScore *= 0.85;

        if (maxVirtualRensaResultScore < rensaScore) {
            maxVirtualRensaResultScore = rensaScore;
        }
    };

    RensaDetector::iteratePossibleRensasIteratively(plan.field(), maxIteration, callback);

    if (maxRensaScoreCollector.get())
        sc_->merge(*maxRensaScoreCollector);
    sc_->setEstimatedRensaScore(maxVirtualRensaResultScore);
}

template class Evaluator<FeatureScoreCollector>;
template class Evaluator<NormalScoreCollector>;
template class RensaEvaluator<FeatureScoreCollector>;
template class RensaEvaluator<NormalScoreCollector>;
