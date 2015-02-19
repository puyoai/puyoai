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
#include "core/algorithm/rensa_detector.h"
#include "core/constant.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "core/field_bit_field.h"
#include "core/position.h"
#include "core/rensa_result.h"
#include "core/score.h"

#include "opening_book.h"
#include "evaluation_parameter.h"
#include "gazer.h"

using namespace std;

DEFINE_bool(pattern, true, "use pattern book");
DEFINE_bool(opening, true, "use opening book");

namespace {

const bool USE_CONNECTION_FEATURE = true;
const bool USE_RESTRICTED_CONNECTION_HORIZONTAL_FEATURE = true;
const bool USE_HAND_WIDTH_FEATURE = true;
const bool USE_THIRD_COLUMN_HEIGHT_FEATURE = true;
const bool USE_IGNITION_HEIGHT_FEATURE = true;
const bool USE_FIELD_USHAPE_FEATURE = true;
const bool USE_RIDGE_FEATURE = true;
const bool USE_VALLEY_FEATURE = true;
const bool USE_FIRE_POINT_TABOO_FEATURE = true;
const bool USE_IBARA = true;

const double FIELD_USHAPE_HEIGHT_COEF[15] = {
    0.0, 0.1, 0.1, 0.3, 0.3,
    0.5, 0.5, 0.7, 0.7, 1.0,
    1.0, 1.0, 1.0, 1.0, 1.0,
};

}

template<typename ScoreCollector>
static void calculateConnection(ScoreCollector* sc, const CoreField& field,
                                EvaluationFeatureKey key2, EvaluationFeatureKey key3)
{
    FieldBitField checked;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        int height = field.height(x);
        for (int y = 1; y <= height; ++y) {
            if (!isNormalColor(field.color(x, y)))
                continue;
            if (checked.get(x, y))
                continue;

            int numConnected = field.countConnectedPuyos(x, y, &checked);
            if (numConnected >= 3) {
                sc->addScore(key3, 1);
            } else if (numConnected >= 2) {
                sc->addScore(key2, 1);
            }
        }
    }
}

template<typename ScoreCollector>
static void calculateValleyDepth(ScoreCollector* sc, EvaluationSparseFeatureKey key, const CoreField& field)
{
    for (int x = 1; x <= 6; ++x) {
        int currentHeight = field.height(x);
        int leftHeight = (x == 1) ? 14 : field.height(x - 1);
        int rightHeight = (x == 6) ? 14 : field.height(x + 1);

        int left = std::max(leftHeight - currentHeight, 0);
        int right = std::max(rightHeight - currentHeight, 0);
        int depth = std::min(left, right);
        DCHECK(0 <= depth && depth <= 14) << depth;
        sc->addScore(key, depth, 1);
    }
}

template<typename ScoreCollector>
static void calculateRidgeHeight(ScoreCollector* sc, EvaluationSparseFeatureKey key, const CoreField& field)
{
    for (int x = 1; x <= 6; ++x) {
        int currentHeight = field.height(x);
        int leftHeight = (x == 1) ? 14 : field.height(x - 1);
        int rightHeight = (x == 6) ? 14 : field.height(x + 1);

        int left = std::max(currentHeight - leftHeight, 0);
        int right = std::max(currentHeight - rightHeight, 0);
        int height = std::min(left, right);
        DCHECK(0 <= height && height <= 14) << height;
        sc->addScore(key, height, 1);
    }
}

template<typename ScoreCollector>
static void calculateFieldUShape(ScoreCollector* sc,
                                 EvaluationFeatureKey linearKey,
                                 EvaluationFeatureKey squareKey,
                                 EvaluationFeatureKey expKey,
                                 bool enemyHasZenkeshi,
                                 const CoreField& field)
{
    static const int DIFF[CoreField::MAP_WIDTH] = {
        0, -3, 0, 1, 1, 0, -3, 0,
    };

    static const int DIFF_ON_ZENKESHI[FieldConstant::MAP_WIDTH] = {
        0, 2, 2, 2, -8, -6, -6, 0
    };

    const int* diff = enemyHasZenkeshi ? DIFF_ON_ZENKESHI : DIFF;

    double average = 0;
    for (int x = 1; x <= 6; ++x)
        average += (field.height(x) + diff[x]);
    average /= 6;

    double linearValue = 0;
    double squareValue = 0;
    double expValue = 0;

    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        int h = field.height(x) + diff[x];
        double coef = FIELD_USHAPE_HEIGHT_COEF[field.height(x)];
        linearValue += std::abs(h - average) * coef;
        squareValue += (h - average) * (h - average) * coef;
        expValue += std::exp(std::abs(h - average)) * coef;
    }

    sc->addScore(linearKey, linearValue);
    sc->addScore(squareKey, squareValue);
    sc->addScore(expKey, expValue);
}

// ----------------------------------------------------------------------

PreEvalResult PreEvaluator::preEval(const CoreField& currentField)
{
    PreEvalResult preEvalResult;

    auto matchableOpeningIds = preEvalResult.mutableMatchableOpeningIds();
    for (size_t i = 0; i < openingBook().size(); ++i) {
        const OpeningBookField& obf = openingBook().field(i);
        if (obf.preMatch(currentField))
            matchableOpeningIds->push_back(static_cast<int>(i));
    }

    auto matchablePatternIds = preEvalResult.mutableMatchablePatternIds();
    ColumnPuyoList cpl;
    for (size_t i = 0; i < patternBook().size(); ++i) {
        const PatternBookField& pbf = patternBook().field(i);
        if (pbf.isMatchable(currentField))
            matchablePatternIds->push_back(static_cast<int>(i));
    }

    return preEvalResult;
}

MidEvalResult MidEvaluator::eval(const RefPlan& plan, const CoreField& currentField, double score)
{
    UNUSED_VARIABLE(currentField);

    MidEvalResult result;
    if (plan.isRensaPlan())
        result.add(MIDEVAL_ERASE, 1);

    result.add(MIDEVAL_RESULT, score);
    return result;
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalBook(const OpeningBook& openingBook,
                                         const std::vector<int>& matchableBookIds,
                                         const RefPlan& plan)
{
    double maxScore = 0;
    const OpeningBookField* bestBf = nullptr;
    set<string> matchedBookNames;

    int totalPuyoCount = plan.field().countPuyos();
    if (totalPuyoCount == 0)
        return;

    for (int id : matchableBookIds) {
        const auto& bf = openingBook.field(id);
        PatternMatchResult mr = bf.match(plan.field());
        if (!mr.matched || mr.count == 0)
            continue;

        matchedBookNames.insert(bf.name());

        // TODO(mayah): How do we handle 'allowedCount' ?
        // 'allowed' cell can be considered as 'matched', however, we'd like to have penalty about it?
        // Some cells should have penalty, however, other cells should not have penalty.

        // TODO(mayah): cutoff ratio should be set to each book?

        double ratio = static_cast<double>(mr.count) / totalPuyoCount;
        DCHECK(0 <= ratio && ratio <= 1.0) << ratio;
        // TODO(mayah): Make this configurable?
        const double cutoffRatio = 0.39;
        if (ratio < cutoffRatio)
            continue;
        ratio = (ratio - 1) / (1 - cutoffRatio) + 1;

        double score = mr.score * ratio;
        if (maxScore < score) {
            bestBf = &bf;
            maxScore = score;
        }
    }

    if (bestBf) {
        sc_->setBookName(bestBf->name());
        sc_->addScore(BOOK, maxScore);
        sc_->addScore(BOOK_KIND, matchedBookNames.size());
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
    calculateConnection(sc_, field, CONNECTION_2, CONNECTION_3);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalRestrictedConnectionHorizontalFeature(const CoreField& f)
{
    const int MAX_HEIGHT = 3; // instead of CoreField::HEIGHT
    for (int y = 1; y <= MAX_HEIGHT; ++y) {
        for (int x = 1; x < CoreField::WIDTH; ++x) {
            if (!isNormalColor(f.color(x, y)))
                continue;

            int len = 1;
            while (f.color(x, y) == f.color(x + len, y))
                ++len;

            EvaluationFeatureKey key;
            if (len == 1) {
                continue;
            } else if (len == 2) {
                if (x <= 3 && 4 < x + len) {
                    key = CONNECTION_HORIZONTAL_CROSSED_2;
                } else {
                    key = CONNECTION_HORIZONTAL_2;
                }
            } else if (len == 3) {
                if (x <= 3 && 4 < x + len) {
                    key = CONNECTION_HORIZONTAL_CROSSED_3;
                } else {
                    key = CONNECTION_HORIZONTAL_3;
                }
            } else {
                CHECK(false) << "shouldn't happen: " << len;
            }

            sc_->addScore(key, 1);
            x += len - 1;
        }
    }
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalThirdColumnHeightFeature(const RefPlan& plan)
{
    sc_->addScore(THIRD_COLUMN_HEIGHT, plan.field().height(3), 1);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalValleyDepth(const CoreField& field)
{
    calculateValleyDepth(sc_, VALLEY_DEPTH, field);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalRidgeHeight(const CoreField& field)
{
    calculateRidgeHeight(sc_, RIDGE_HEIGHT, field);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalFieldUShape(const CoreField& field, bool enemyHasZenkeshi)
{
    if (enemyHasZenkeshi) {
        calculateFieldUShape(sc_,
                             FIELD_USHAPE_LINEAR_ON_ZENKESHI,
                             FIELD_USHAPE_SQUARE_ON_ZENKESHI,
                             FIELD_USHAPE_EXP_ON_ZENKESHI,
                             true,
                             field);
    } else {
        calculateFieldUShape(sc_,
                             FIELD_USHAPE_LINEAR,
                             FIELD_USHAPE_SQUARE,
                             FIELD_USHAPE_EXP,
                             false,
                             field);
    }
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalUnreachableSpace(const CoreField& f)
{
    FieldBitField checked;
    f.countConnectedPuyos(3, 12, &checked);

    int countUnreachable = 0;
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = f.height(x) + 1; y <= CoreField::HEIGHT; ++y) {
            if (f.color(x, y) != PuyoColor::EMPTY)
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
bool Evaluator<ScoreCollector>::evalStrategy(const RefPlan& plan, const CoreField& currentField, int currentFrameId,
                                             const PlayerState& me, const PlayerState& enemy, const GazeResult& gazeResult)
{
    if (!plan.isRensaPlan())
        return false;

    bool inTime = false;
    {
        if (plan.decisions().size() == 1) {
            // Sometimes, enemy.finishingRensaFrameId might be wrong.
            // So, if plan.decisions.size() == 1, we always consider it's in time.
            inTime = true;
        } else if (me.fixedOjama > 0) {
            // If fixedOjama > 0, after our first hand, ojama will be dropped.
            // So it's not in time.
            inTime = false;
        } else if (!enemy.isRensaOngoing) {
            // If enemy is not firing rensa, we can think in time.
            inTime = true;
        } else if (currentFrameId + plan.framesToIgnite() < enemy.finishingRensaFrameId) {
            // If we can play before finishing enemy's rensa, it's in time.
            inTime = true;
        } else {
            // Otherwise, it's not in time.
            inTime = false;
        }
    }

    // If not in time, we cannot fire a rensa. So considering firing rensa is meaning less.
    if (!inTime) {
        return false;
    }

    if (enemy.isRensaOngoing && me.fixedOjama + me.pendingOjama >= 6) {
        if (plan.score() >= scoreForOjama(std::max(0, me.fixedOjama + me.pendingOjama - 3))) {
            sc_->addScore(STRATEGY_TAIOU, 1.0);
            return false;
        }
    }

    if (plan.field().isZenkeshi()) {
        int puyoCount = plan.decisions().size() * 2 + currentField.countPuyos();
        if (puyoCount <= 16) {
            sc_->addScore(STRATEGY_SCORE, plan.score());
            sc_->addScore(STRATEGY_INITIAL_ZENKESHI, 1);
            return true;
        }
        sc_->addScore(STRATEGY_SCORE, plan.score());
        sc_->addScore(STRATEGY_ZENKESHI, 1);
        return true;
    }

    if (me.hasZenkeshi && !enemy.hasZenkeshi) {
        if (!enemy.isRensaOngoing) {
            sc_->addScore(STRATEGY_SCORE, plan.score());
            sc_->addScore(STRATEGY_ZENKESHI_CONSUME, 1);
            return false;
        }
        if (me.pendingOjama + me.fixedOjama <= 36) {
            sc_->addScore(STRATEGY_SCORE, plan.score());
            sc_->addScore(STRATEGY_ZENKESHI_CONSUME, 1);
            return false;
        }
    }

    int rensaEndingFrameId = currentFrameId + plan.totalFrames();
    int estimatedMaxScore = gazeResult.estimateMaxScore(rensaEndingFrameId, enemy);

    // --- If the rensa is large enough, fire it.
    if (plan.score() >= estimatedMaxScore + scoreForOjama(60)) {
        sc_->addScore(STRATEGY_LARGE_ENOUGH, 1);
        return true;
    }

    sc_->addScore(STRATEGY_SCORE, plan.score());

    // If IBARA found, we always consider it.
    // TODO(mayah): Don't consider IBARA if we don't have enough puyos. Better not to fire IBARA in that case.
    if (USE_IBARA && plan.chains() == 1 && plan.score() >= scoreForOjama(10) && me.pendingOjama + me.fixedOjama <= 10) {
        sc_->addScore(STRATEGY_IBARA, 1);
        return false;
    }

    // If we can send 18>= ojamas, and opponent does not have any hand to cope with it, we can fire it.
    // TODO(mayah): We need to check if the enemy cannot fire his rensa after ojama is dropped.
    if (plan.chains() <= 3 && plan.score() >= scoreForOjama(15) &&
        me.pendingOjama + me.fixedOjama <= 3 && estimatedMaxScore <= scoreForOjama(12)) {
        sc_->addScore(STRATEGY_TSUBUSHI, 1);
        return true;
    }

    sc_->addScore(STRATEGY_SAKIUCHI, 1.0);

    // TODO(mayah): Check land leveling.
    // TODO(mayah): OIUCHI?
    // TODO(mayah): KILL when enemy has a lot of puyos?
    return false;
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaStrategy(const RefPlan& plan, const RensaResult& rensaResult,
                                                       const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                                                       int currentFrameId,
                                                       const PlayerState& me, const PlayerState& enemy)
{
    UNUSED_VARIABLE(currentFrameId);
    UNUSED_VARIABLE(me);

    if (plan.field().countPuyos() >= 36 && plan.score() >= scoreForOjama(15) && plan.chains() <= 3 && rensaResult.chains >= 7 &&
        keyPuyos.size() + firePuyos.size() <= 3 && !enemy.isRensaOngoing) {
        sc_->addScore(STRATEGY_SAISOKU, 1);
    }
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaChainFeature(const RensaResult& rensaResult,
                                                           const PuyoSet& totalPuyoSet)
{
    sc_->addScore(MAX_CHAINS, rensaResult.chains, 1);

    int totalNecessaryPuyos = TsumoPossibility::necessaryPuyos(totalPuyoSet, 0.5);
    sc_->addScore(NECESSARY_PUYOS_LINEAR, totalNecessaryPuyos);
    sc_->addScore(NECESSARY_PUYOS_SQUARE, totalNecessaryPuyos * totalNecessaryPuyos);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaHandWidthFeature(const CoreField& field,
                                                               const RensaTrackResult& trackResult)
{
    static const int dx[4] = { 0,  0, 1, -1 };
    static const int dy[4] = { 1, -1, 0,  0 };

    int distanceCount[5] {};
    int distance[CoreField::MAP_WIDTH][CoreField::MAP_HEIGHT] {};

    // TODO(mayah): Using std::queue is 2x slower here.
    Position q[CoreField::MAP_WIDTH * CoreField::MAP_HEIGHT];
    Position* qHead = q;
    Position* qTail = q;

    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        int h = field.height(x);
        for (int y = 1; y <= h; ++y) {
            DCHECK(field.color(x, y) != PuyoColor::EMPTY);
            if (trackResult.erasedAt(x, y) != 1)
                continue;
            if (field.color(x, y + 1) == PuyoColor::EMPTY && trackResult.erasedAt(x, y + 1) == 0)
                continue;

            distanceCount[1]++;
            distance[x][y] = 1;
            *qTail++ = Position(x, y);
            break;
        }
    }

    while (qHead != qTail) {
        const int x = qHead->x;
        const int y = qHead->y;
        qHead++;

        for (int i = 0; i < 4; ++i) {
            const int xx = x + dx[i];
            const int yy = y + dy[i];

            if (field.color(xx, yy) == PuyoColor::EMPTY)
                continue;
            if (trackResult.erasedAt(xx, yy) != 1)
                continue;
            if (distance[xx][yy] != 0)
                continue;

            *qTail++ = Position(xx, yy);
            distanceCount[1]++;
            distance[xx][yy] = 1;
        }
    }

    qHead = q;

    while (qHead != qTail) {
        const int x = qHead->x;
        const int y = qHead->y;
        qHead++;

        int d = distance[x][y] + 1;

        for (int i = 0; i < 4; ++i) {
            const int xx = x + dx[i];
            const int yy = y + dy[i];

            if (distance[xx][yy] != 0)
                continue;
            if (field.color(xx, yy) != PuyoColor::EMPTY)
                continue;
            if (trackResult.erasedAt(xx, yy) >= 2)
                continue;

            distance[xx][yy] = d;
            distanceCount[d]++;
            if (d <= 3)
                *qTail++ = Position(xx, yy);
        }
    }

    if (distanceCount[2] > 0)
        sc_->addScore(HAND_WIDTH_2, distanceCount[2] > 10 ? 10 : distanceCount[2], 1);
    if (distanceCount[3] > 0)
        sc_->addScore(HAND_WIDTH_3, distanceCount[3] > 10 ? 10 : distanceCount[3], 1);
    if (distanceCount[4] > 0)
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
    calculateConnection(sc_, fieldAfterDrop, CONNECTION_AFTER_DROP_2, CONNECTION_AFTER_DROP_3);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaScore(double score, double virtualScore)
{
    sc_->addScore(SCORE, score);
    sc_->addScore(VIRTUAL_SCORE, virtualScore);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaRidgeHeight(const CoreField& field)
{
    calculateRidgeHeight(sc_, RENSA_RIDGE_HEIGHT, field);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaValleyDepth(const CoreField& field)
{
    calculateValleyDepth(sc_, RENSA_VALLEY_DEPTH, field);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaFieldUShape(const CoreField& field, bool enemyHasZenkeshi)
{
    if (enemyHasZenkeshi) {
        calculateFieldUShape(sc_,
                             RENSA_FIELD_USHAPE_LINEAR_ON_ZENKESHI,
                             RENSA_FIELD_USHAPE_SQUARE_ON_ZENKESHI,
                             RENSA_FIELD_USHAPE_EXP_ON_ZENKESHI,
                             true,
                             field);
    } else {
        calculateFieldUShape(sc_,
                             RENSA_FIELD_USHAPE_LINEAR,
                             RENSA_FIELD_USHAPE_SQUARE,
                             RENSA_FIELD_USHAPE_EXP,
                             false,
                             field);
    }
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
void Evaluator<ScoreCollector>::evalMidEval(const MidEvalResult& midEvalResult)
{
    // Copy midEvalResult.
    for (const auto& entry : midEvalResult.collectedFeatures()) {
        sc_->addScore(entry.first, entry.second);
    }
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::collectScore(const RefPlan& plan, const CoreField& currentField,
                                             int currentFrameId, int maxIteration,
                                             const PlayerState& me,
                                             const PlayerState& enemy,
                                             const PreEvalResult& preEvalResult,
                                             const MidEvalResult& midEvalResult,
                                             const GazeResult& gazeResult)
{
    const CoreField& fieldBeforeRensa = plan.field();

    evalMidEval(midEvalResult);

    // We'd like to evaluate frame feature always.
    evalFrameFeature(plan);

    if (evalStrategy(plan, currentField, currentFrameId, me, enemy, gazeResult))
        return;

    if (FLAGS_opening && !enemy.hasZenkeshi && !plan.isRensaPlan()) {
        evalBook(openingBook(), preEvalResult.matchableOpeningIds(), plan);
    }
    evalCountPuyoFeature(plan);
    if (USE_CONNECTION_FEATURE)
        collectScoreForConnection(fieldBeforeRensa);
    if (USE_RESTRICTED_CONNECTION_HORIZONTAL_FEATURE)
        evalRestrictedConnectionHorizontalFeature(fieldBeforeRensa);
    if (USE_THIRD_COLUMN_HEIGHT_FEATURE)
        evalThirdColumnHeightFeature(plan);
    if (USE_VALLEY_FEATURE)
        evalValleyDepth(fieldBeforeRensa);
    if (USE_RIDGE_FEATURE)
        evalRidgeHeight(fieldBeforeRensa);
    if (USE_FIELD_USHAPE_FEATURE)
        evalFieldUShape(plan.field(), enemy.hasZenkeshi);

    evalUnreachableSpace(fieldBeforeRensa);

    bool hasSideChain = false;
    int numReachableSpace = fieldBeforeRensa.countConnectedPuyos(3, 12);
    int maxVirtualRensaResultScore = 0;
    double maxRensaScore = -100000000; // TODO(mayah): Should be negative infty?
    ColumnPuyoList maxRensaKeyPuyos;
    ColumnPuyoList maxRensaFirePuyos;
    std::unique_ptr<ScoreCollector> maxRensaScoreCollector;
    auto evalCallback = [&](const CoreField& fieldBeforeRensa, const CoreField& fieldAfterRensa,
                            const RensaResult& rensaResult,
                            const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                            const RensaTrackResult& trackResult) {
        std::unique_ptr<ScoreCollector> rensaScoreCollector(new ScoreCollector(sc_->evaluationParameter()));
        RensaEvaluator<ScoreCollector> rensaEvaluator(openingBook(), patternBook(), rensaScoreCollector.get());

        CoreField complementedField(fieldBeforeRensa);
        for (const auto& cp : keyPuyos)
            complementedField.dropPuyoOn(cp.x, cp.color);
        for (const auto& cp : firePuyos)
            complementedField.dropPuyoOn(cp.x, cp.color);

        rensaEvaluator.evalRensaRidgeHeight(complementedField);
        rensaEvaluator.evalRensaValleyDepth(complementedField);
        rensaEvaluator.evalRensaFieldUShape(complementedField, enemy.hasZenkeshi);

        if (keyPuyos.size() == 0 && rensaResult.chains == 2 && rensaResult.score >= 1000) {
            hasSideChain = true;
        }

        PuyoSet necessaryPuyos;
        necessaryPuyos.add(keyPuyos);
        necessaryPuyos.add(firePuyos);

        rensaEvaluator.evalRensaChainFeature(rensaResult, necessaryPuyos);
        rensaEvaluator.collectScoreForRensaGarbage(fieldAfterRensa);
        if (USE_HAND_WIDTH_FEATURE)
            rensaEvaluator.evalRensaHandWidthFeature(fieldBeforeRensa, trackResult);
        if (USE_FIRE_POINT_TABOO_FEATURE)
            rensaEvaluator.evalFirePointTabooFeature(plan, trackResult);
        if (USE_IGNITION_HEIGHT_FEATURE)
            rensaEvaluator.evalRensaIgnitionHeightFeature(plan, trackResult, enemy.hasZenkeshi);
        if (USE_CONNECTION_FEATURE)
            rensaEvaluator.evalRensaConnectionFeature(fieldAfterRensa);

        rensaEvaluator.evalRensaStrategy(plan, rensaResult, keyPuyos, firePuyos, currentFrameId, me, enemy);

        if (rensaScoreCollector->score() > maxRensaScore) {
            maxRensaScore = rensaScoreCollector->score();
            maxRensaScoreCollector = move(rensaScoreCollector);
            maxRensaKeyPuyos = keyPuyos;
            maxRensaFirePuyos = firePuyos;
        }

        const double rensaScore = rensaResult.score;
        double possibility = TsumoPossibility::possibility(necessaryPuyos, std::max(0, numReachableSpace));
        // possibility = std::min(1.0, possibility + 0.15);

        const double virtualRensaScore = rensaScore * possibility;
        rensaEvaluator.evalRensaScore(rensaScore, virtualRensaScore);

        if (maxVirtualRensaResultScore < virtualRensaScore) {
            maxVirtualRensaResultScore = virtualRensaScore;
        }
    };

    {
        RensaDetectorStrategy strategy(RensaDetectorStrategy::Mode::DROP, 2, 3, false);
        auto callback = [&](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                            const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                            const RensaTrackResult& trackResult, const RensaRefSequence&) {
            evalCallback(fieldBeforeRensa, fieldAfterRensa, rensaResult, keyPuyos, firePuyos, trackResult);
        };
        RensaDetector::iteratePossibleRensasIteratively(fieldBeforeRensa, maxIteration, strategy, callback);
    }

    if (FLAGS_pattern) {
        RensaDetectorStrategy strategy(RensaDetectorStrategy::Mode::DROP, 2, 1, false);
        for (const int id : preEvalResult.matchablePatternIds()) {
            const PatternBookField& pbf = patternBook().field(id);
            ColumnPuyoList cpl;
            if (!pbf.complement(plan.field(), &cpl))
                continue;
            if (cpl.isEmpty())
                continue;

            auto callback = [&](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                                const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                                const RensaTrackResult& trackResult, const RensaRefSequence&) {
                if (keyPuyos.size() + cpl.size() > ColumnPuyoList::MAX_SIZE)
                    return;
                ColumnPuyoList allKeyPuyos;
                allKeyPuyos.append(cpl);
                allKeyPuyos.append(keyPuyos);
                evalCallback(plan.field(), fieldAfterRensa, rensaResult, allKeyPuyos, firePuyos, trackResult);
            };
            CoreField complementedField(plan.field());
            for (const auto& cp : cpl)
                complementedField.dropPuyoOn(cp.x, cp.color);
            RensaDetector::iteratePossibleRensasIteratively(complementedField, maxIteration, strategy, callback);
        }
    }

    if (hasSideChain) {
        sc_->addScore(HOLDING_SIDE_CHAIN, 1.0);
    }

    if (maxRensaScoreCollector.get()) {
        sc_->merge(*maxRensaScoreCollector);
        sc_->setRensaKeyPuyos(maxRensaKeyPuyos);
        sc_->setRensaFirePuyos(maxRensaFirePuyos);
    }
    sc_->setEstimatedRensaScore(maxVirtualRensaResultScore);
}

template class Evaluator<FeatureScoreCollector>;
template class Evaluator<NormalScoreCollector>;
template class RensaEvaluator<FeatureScoreCollector>;
template class RensaEvaluator<NormalScoreCollector>;
