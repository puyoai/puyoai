#ifndef CPU_MAYAH_EVALUATOR_INL_H_
#define CPU_MAYAH_EVALUATOR_INL_H_

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>

#include <glog/logging.h>

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
#include "util.h"

const bool USE_CONNECTION_FEATURE = true;
const bool USE_CONNECTION_HORIZONTAL_FEATURE = false;
const bool USE_RESTRICTED_CONNECTION_HORIZONTAL_FEATURE = true;
const bool USE_HAND_WIDTH_FEATURE = true;
const bool USE_HEIGHT_DIFF_FEATURE = false;
const bool USE_THIRD_COLUMN_HEIGHT_FEATURE = true;
const bool USE_DENSITY_FEATURE = false;
const bool USE_IGNITION_HEIGHT_FEATURE = true;
const bool USE_FIELD_USHAPE_FEATURE = true;

template<typename ScoreCollector>
void evalBook(ScoreCollector* sc, const std::vector<BookField>& books, const RefPlan& plan)
{
    double maxScore = 0;
    const BookField* bestBf = nullptr;

    double totalPuyoCount = plan.field().countPuyos();
    for (const auto& bf : books) {
        double score = bf.match(plan.field()) / totalPuyoCount;
        // double score = bf.match(plan.field());
        if (maxScore < score) {
            bestBf = &bf;
            maxScore = score;
        }
    }

    if (bestBf) {
        sc->addScore(BOOK, maxScore);
        sc->setBookName(bestBf->name());
    }
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

template<typename ScoreCollector>
void collectScoreForConnection(ScoreCollector* sc, const CoreField& field)
{
    calculateConnection(sc, field, CONNECTION);
}

template<typename ScoreCollector>
void evalConnectionHorizontalFeature(ScoreCollector* sc, const RefPlan& plan)
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
            sc->addScore(CONNECTION_HORIZONTAL, len, 1);
            x += len - 1;
        }
    }
}

template<typename ScoreCollector>
void evalRestrictedConnectionHorizontalFeature(ScoreCollector* sc, const RefPlan& plan)
{
    EvaluationSparseFeatureKey keys[] = {
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

            sc->addScore(keys[x], len, 1);
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
                PuyoColor c = NORMAL_PUYO_COLORS[i];
                sc->addScore(DENSITY, c, 1);
            }
        }
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
        double diff = std::abs(field.height(x) - averageHeight);
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
            int left = std::max(leftHeight - currentHeight, 0);
            int right = std::max(rightHeight - currentHeight, 0);
            int depth = std::min(left, right);
#if 0
            if (x == 1 || x == 6) {
                if (depth > 0)
                    depth -= 1;
            }
#endif
            DCHECK(0 <= depth && depth <= 14) << depth;
            sc->addScore(VALLEY_DEPTH, depth, 1);
        }

        // --- ridge
        {
            int left = std::max(currentHeight - leftHeight, 0);
            int right = std::max(currentHeight - rightHeight, 0);
            int height = std::min(left, right);
            DCHECK(0 <= height && height <= 14) << height;
            sc->addScore(RIDGE_HEIGHT, height, 1);
        }
    }
}

template<typename ScoreCollector>
void evalFieldUShape(ScoreCollector* sc, const RefPlan& plan)
{
    static int DIFF[CoreField::MAP_WIDTH] = {
        0, -3, 0, 1, 1, 0, -3, 0,
    };

    const CoreField& f = plan.field();
    int sumHeight = 0;
    for (int x = 1; x <= 6; ++x)
        sumHeight += f.height(x);

    double s = 0;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        if (f.height(x) <= 4) {
            s += 0.06 * std::abs((f.height(x) + DIFF[x]) * 6 - sumHeight);
        } else {
            s += std::abs((f.height(x) + DIFF[x]) * 6 - sumHeight);
        }
    }

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

    if (currentField.countPuyos() >= 55) {
        sc->addScore(STRATEGY_HOUWA, 1);
        return false;
    }

    sc->addScore(STRATEGY_SAKIUCHI, 1.0);
    return false;
}

template<typename ScoreCollector>
void evalRensaStrategy(ScoreCollector* sc, const RefPlan& plan, const RensaResult& rensaResult,
                       const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                       int currentFrameId, const Gazer& gazer)
{
    UNUSED_VARIABLE(currentFrameId);

    // TODO(mayah): Ah, maybe sakiuchi etc. wins this value?
    if (plan.field().countPuyos() >= 36 && plan.score() >= scoreForOjama(15) && rensaResult.chains >= 7 &&
        keyPuyos.size() + firePuyos.size() <= 3 && !gazer.rensaIsOngoing()) {
        sc->addScore(STRATEGY_SAISOKU, 1);
    }
}

template<typename ScoreCollector>
void evalRensaChainFeature(ScoreCollector* sc, const RefPlan& plan,
                           const RensaResult& rensaResult,
                           const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos)
{
    int numKeyPuyos = std::min(7, static_cast<int>(keyPuyos.size()));
    int numFirePuyos = std::min(7, static_cast<int>(firePuyos.size()));

    sc->addScore(MAX_CHAINS, rensaResult.chains, 1);
    if (plan.field().countPuyos() <= 24) {
        sc->addScore(MAX_RENSA_KEY_PUYOS_EARLY, numKeyPuyos, 1);
        sc->addScore(MAX_RENSA_FIRE_PUYOS_EARLY, numFirePuyos, 1);
    } else if (plan.field().countPuyos() <= 42) {
        sc->addScore(MAX_RENSA_KEY_PUYOS_MIDDLE, numKeyPuyos, 1);
        sc->addScore(MAX_RENSA_FIRE_PUYOS_MIDDLE, numFirePuyos, 1);
    } else {
        sc->addScore(MAX_RENSA_KEY_PUYOS_LATE, numKeyPuyos, 1);
        sc->addScore(MAX_RENSA_FIRE_PUYOS_LATE, numFirePuyos, 1);
    }
}

template<typename ScoreCollector>
void evalRensaHandWidthFeature(ScoreCollector* sc, const RefPlan& plan, const RensaTrackResult& trackResult)
{
    const CoreField& field = plan.field();

    int distanceCount[5] = { 0, 0, 0, 0, 0 };
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

        if (distance[x][y] > 4)
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

#if 0
    std::cout << plan.field().toDebugString() << std::endl;
    for (int y = 12; y >= 1; --y) {
        for (int x = 1; x <= 6; ++x) {
            std::cout << distance[x][y] << ' ';
        }
        std::cout << std::endl;
    }
#endif

    sc->addScore(HAND_WIDTH_2, distanceCount[2] > 10 ? 10 : distanceCount[2], 1);
    sc->addScore(HAND_WIDTH_3, distanceCount[3] > 10 ? 10 : distanceCount[3], 1);
    sc->addScore(HAND_WIDTH_4, distanceCount[4] > 10 ? 10 : distanceCount[4], 1);
}

template<typename ScoreCollector>
void evalRensaIgnitionHeightFeature(ScoreCollector* sc, const RefPlan& plan, const RensaTrackResult& trackResult)
{
    for (int y = CoreField::HEIGHT; y >= 1; --y) {
        for (int x = 1; x <= CoreField::WIDTH; ++x) {
            if (!isNormalColor(plan.field().color(x, y)))
                continue;
            if (trackResult.erasedAt(x, y) == 1) {
                sc->addScore(IGNITION_HEIGHT, y, 1);
                return;
            }
        }
    }

    sc->addScore(IGNITION_HEIGHT, 0, 1);
}

template<typename ScoreCollector>
void evalRensaConnectionFeature(ScoreCollector* sc, const CoreField& fieldAfterDrop)
{
    calculateConnection(sc, fieldAfterDrop, CONNECTION_AFTER_VANISH);
}

template<typename ScoreCollector>
void collectScoreForRensaGarbage(ScoreCollector* sc, const CoreField& fieldAfterDrop)
{
    sc->addScore(NUM_GARBAGE_PUYOS, fieldAfterDrop.countPuyos());
    sc->addScore(NUM_SIDE_GARBAGE_PUYOS, fieldAfterDrop.height(1) + fieldAfterDrop.height(6));
}

template<typename ScoreCollector>
void evalCountPuyoFeature(ScoreCollector* sc, const RefPlan& plan)
{
    sc->addScore(NUM_COUNT_PUYOS, plan.field().countColorPuyos(), 1);
}

template<typename ScoreCollector>
void collectScore(ScoreCollector* sc, const std::vector<BookField>& books, const RefPlan& plan, const CoreField& currentField,
                  int currentFrameId, int maxIteration, const Gazer& gazer)
{
    // We'd like to evaluate frame feature always.
    evalFrameFeature(sc, plan);

    if (evalStrategy(sc, plan, currentField, currentFrameId, gazer))
        return;

    evalBook(sc, books, plan);
    evalCountPuyoFeature(sc, plan);
    if (USE_CONNECTION_FEATURE)
        collectScoreForConnection(sc, plan.field());
    if (USE_CONNECTION_HORIZONTAL_FEATURE)
        evalConnectionHorizontalFeature(sc, plan);
    if (USE_RESTRICTED_CONNECTION_HORIZONTAL_FEATURE)
        evalRestrictedConnectionHorizontalFeature(sc, plan);
    if (USE_DENSITY_FEATURE)
        evalDensityFeature(sc, plan);
    if (USE_HEIGHT_DIFF_FEATURE)
        evalFieldHeightFeature(sc, plan);
    if (USE_THIRD_COLUMN_HEIGHT_FEATURE)
        evalThirdColumnHeightFeature(sc, plan);
    evalValleyDepthRidgeHeight(sc, plan);
    if (USE_FIELD_USHAPE_FEATURE)
        evalFieldUShape(sc, plan);
    evalUnreachableSpace(sc, plan);

    double maxRensaScore = -100000000; // TODO(mayah): Should be negative infty?
    std::unique_ptr<ScoreCollector> maxRensaScoreCollector;
    auto callback = [&](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                        const RensaTrackResult& trackResult, const RensaRefSequence&) {
        std::unique_ptr<ScoreCollector> rensaScoreCollector(new ScoreCollector(sc->featureParameter()));
        evalRensaChainFeature(rensaScoreCollector.get(), plan, rensaResult, keyPuyos, firePuyos);
        collectScoreForRensaGarbage(rensaScoreCollector.get(), fieldAfterRensa);
        if (USE_HAND_WIDTH_FEATURE)
            evalRensaHandWidthFeature(rensaScoreCollector.get(), plan, trackResult);
        if (USE_IGNITION_HEIGHT_FEATURE)
            evalRensaIgnitionHeightFeature(rensaScoreCollector.get(), plan, trackResult);
        if (USE_CONNECTION_FEATURE)
            evalRensaConnectionFeature(rensaScoreCollector.get(), fieldAfterRensa);

        evalRensaStrategy(rensaScoreCollector.get(), plan, rensaResult, keyPuyos, firePuyos, currentFrameId, gazer);

        if (rensaScoreCollector->score() > maxRensaScore) {
            maxRensaScore = rensaScoreCollector->score();
            maxRensaScoreCollector = move(rensaScoreCollector);
        }
    };

    RensaDetector::iteratePossibleRensasIteratively(plan.field(), maxIteration, callback);

    if (maxRensaScoreCollector.get())
        sc->merge(*maxRensaScoreCollector);
}

#endif
