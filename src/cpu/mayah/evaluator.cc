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

#include "evaluation_parameter.h"
#include "gazer.h"
#include "pattern_rensa_detector.h"

using namespace std;

namespace {

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
    int count3 = 0;
    int count2 = 0;

    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        int height = field.height(x);
        for (int y = 1; y <= height; ++y) {
            if (!isNormalColor(field.color(x, y)))
                continue;

            int numConnected = field.countConnectedPuyosMax4(x, y);
            if (numConnected >= 3) {
                ++count3;
            } else if (numConnected >= 2) {
                ++count2;
            }
        }
    }

    sc->addScore(key3, count3 / 3);
    sc->addScore(key2, count2 / 2);
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

    auto matchablePatternIds = preEvalResult.mutableMatchablePatternIds();
    for (size_t i = 0; i < patternBook().size(); ++i) {
        const PatternBookField& pbf = patternBook().patternBookField(i);
        if (pbf.ignitionColumn() == 0)
            continue;
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
void Evaluator<ScoreCollector>::evalFrameFeature(const RefPlan& plan)
{
    sc_->addScore(TOTAL_FRAMES, plan.totalFrames());
    sc_->addScore(NUM_CHIGIRI, plan.numChigiri());
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalConnection(const CoreField& field)
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
    calculateFieldUShape(sc_,
                         FIELD_USHAPE_LINEAR,
                         FIELD_USHAPE_SQUARE,
                         FIELD_USHAPE_EXP,
                         enemyHasZenkeshi,
                         field);
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalUnreachableSpace(const CoreField& f)
{
    int count = 0;

    if (f.height(2) >= 12 && f.height(1) < 12)
        count += 12 - f.height(1);
    if (f.height(4) >= 12 && f.height(5) < 12)
        count += 12 - f.height(5);
    if ((f.height(4) >= 12 || f.height(5) >= 12) && f.height(6) < 12)
        count += 12 - f.height(6);

    sc_->addScore(NUM_UNREACHABLE_SPACE, count);
}

// Returns true If we don't need to evaluate other features.
template<typename ScoreCollector>
bool Evaluator<ScoreCollector>::evalStrategy(const RefPlan& plan, const CoreField& currentField, int currentFrameId,
                                             const PlayerState& me, const PlayerState& enemy, const GazeResult& gazeResult,
                                             const MidEvalResult& midEvalResult)
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

    int rensaEndingFrameId = currentFrameId + plan.totalFrames();
    int estimatedMaxScore = gazeResult.estimateMaxScore(rensaEndingFrameId, enemy);

    if (!enemy.isRensaOngoing && plan.chains() <= 4) {
        int h = 12 - enemy.field.height(3);
        if (plan.score() - estimatedMaxScore >= scoreForOjama(6 * h)) {
            sc_->addScore(STRATEGY_KILL, 1);
            sc_->addScore(STRATEGY_KILL_FRAME, plan.totalFrames());
            return true;
        }
    }

    // --- If the rensa is large enough, fire it.
    if (plan.score() >= estimatedMaxScore + scoreForOjama(60) && !enemy.isRensaOngoing) {
        sc_->addScore(STRATEGY_LARGE_ENOUGH, 1);
        return true;
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

    sc_->addScore(STRATEGY_SCORE, plan.score());

    // If IBARA found, we always consider it.
    // TODO(mayah): Don't consider IBARA if we don't have enough puyos. Better not to fire IBARA in that case.
    if (plan.chains() == 1 && plan.score() >= scoreForOjama(10) && me.pendingOjama + me.fixedOjama <= 10) {
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

    if (plan.chains() <= 3 && me.pendingOjama + me.fixedOjama == 0 && midEvalResult.feature(MIDEVAL_ERASE) == 0) {
        if (plan.score() >= scoreForOjama(30)) {
            sc_->addScore(STRATEGY_FIRE_SIDE_CHAIN_LARGE, 1);
        } else if (plan.score() >= scoreForOjama(18)) {
            sc_->addScore(STRATEGY_FIRE_SIDE_CHAIN_MEDIUM, 1);
        } else if (plan.score() >= scoreForOjama(15)) {
            sc_->addScore(STRATEGY_FIRE_SIDE_CHAIN_SMALL, 1);
        }
        return false;
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
void RensaEvaluator<ScoreCollector>::evalFirePointTabooFeature(const RefPlan& plan, const RensaChainTrackResult& trackResult)
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
void RensaEvaluator<ScoreCollector>::evalRensaIgnitionHeightFeature(const RefPlan& plan, const RensaChainTrackResult& trackResult, bool enemyHasZenkeshi)
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
    calculateFieldUShape(sc_,
                         RENSA_FIELD_USHAPE_LINEAR,
                         RENSA_FIELD_USHAPE_SQUARE,
                         RENSA_FIELD_USHAPE_EXP,
                         enemyHasZenkeshi,
                         field);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalPatternScore(double patternScore)
{
    sc_->addScore(PATTERN_BOOK, patternScore);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalComplementationBias(const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos)
{
    for (int x = 1; x <= 6; ++x) {
        PuyoSet ps;
        int h = keyPuyos.sizeOn(x);
        for (int i = 0; i < h; ++i)
            ps.add(keyPuyos.get(x, i));

        h = firePuyos.sizeOn(x);
        for (int i = 0; i < h; ++i)
            ps.add(firePuyos.get(x, i));

        if (ps.count() < 3)
            continue;
        if (ps.count() >= 4)
            sc_->addScore(COMPLEMENTATION_BIAS_MUCH, 1);
        if (ps.red() >= 3 || ps.blue() >= 3 || ps.yellow() >= 3 || ps.green() >= 3) {
            EvaluationFeatureKey key = (x == 1 || x == 6) ? COMPLEMENTATION_BIAS_EDGE : COMPLEMENTATION_BIAS;
            sc_->addScore(key, 1);
        }
    }
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaGarbage(const CoreField& fieldAfterDrop)
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
EvaluationMode Evaluator<ScoreCollector>::calculateMode(const PlayerState& me, const PlayerState& enemy) const
{
    const int EARLY_THRESHOLD = 24;
    const int EARLY_MIDDLE_THRESHOLD = 36;
    const int MIDDLE_THRESHOLD = 54;

    if (enemy.field.isZenkeshi())
        return EvaluationMode::ENEMY_HAS_ZENKESHI;

    int count = me.field.countPuyos();
    if (count <= EARLY_THRESHOLD)
        return EvaluationMode::EARLY;
    if (count <= EARLY_MIDDLE_THRESHOLD)
        return EvaluationMode::EARLY_MIDDLE;
    if (count <= MIDDLE_THRESHOLD)
        return EvaluationMode::MIDDLE;

    return EvaluationMode::LATE;
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::eval(const RefPlan& plan, const CoreField& currentField,
                                     int currentFrameId, int maxIteration,
                                     const PlayerState& me,
                                     const PlayerState& enemy,
                                     const PreEvalResult& preEvalResult,
                                     const MidEvalResult& midEvalResult,
                                     const GazeResult& gazeResult)
{
    EvaluationMode mode = calculateMode(me, enemy);
    sc_->setMode(mode);

    const CoreField& fieldBeforeRensa = plan.field();

    evalMidEval(midEvalResult);

    // We'd like to evaluate frame feature always.
    evalFrameFeature(plan);

    if (evalStrategy(plan, currentField, currentFrameId, me, enemy, gazeResult, midEvalResult))
        return;

    evalCountPuyoFeature(plan);
    evalConnection(fieldBeforeRensa);
    evalRestrictedConnectionHorizontalFeature(fieldBeforeRensa);
    evalThirdColumnHeightFeature(plan);
    evalValleyDepth(fieldBeforeRensa);
    evalRidgeHeight(fieldBeforeRensa);
    evalFieldUShape(plan.field(), enemy.hasZenkeshi);

    evalUnreachableSpace(fieldBeforeRensa);

    int sideChainMaxScore = 0;
    int numReachableSpace = fieldBeforeRensa.countConnectedPuyos(3, 12);
    int fastChainMaxScore = 0;
    int maxVirtualRensaResultScore = 0;
    double maxRensaScore = -100000000; // TODO(mayah): Should be negative infty?
    ColumnPuyoList maxRensaKeyPuyos;
    ColumnPuyoList maxRensaFirePuyos;
    std::unique_ptr<ScoreCollector> maxRensaScoreCollector;
    int rensaCounts[20] {};
    int maxScoreChains = 0;
    auto evalCallback = [&](const CoreField& fieldBeforeRensa,
                            const CoreField& fieldAfterRensa,
                            const RensaResult& rensaResult,
                            const ColumnPuyoList& keyPuyos,
                            const ColumnPuyoList& firePuyos,
                            double patternScore,
                            const string& patternName,
                            const RensaChainTrackResult& trackResult) {
        ++rensaCounts[rensaResult.chains];

        std::unique_ptr<ScoreCollector> rensaScoreCollector(new ScoreCollector(sc_->evaluationParameterMap()));
        RensaEvaluator<ScoreCollector> rensaEvaluator(patternBook(), rensaScoreCollector.get());

        rensaScoreCollector->setBookName(patternName);

        CoreField complementedField(fieldBeforeRensa);
        if (!complementedField.dropPuyoList(keyPuyos))
            return;
        if (!complementedField.dropPuyoList(firePuyos))
            return;

        rensaEvaluator.evalRensaRidgeHeight(complementedField);
        rensaEvaluator.evalRensaValleyDepth(complementedField);
        rensaEvaluator.evalRensaFieldUShape(complementedField, enemy.hasZenkeshi);
        rensaEvaluator.evalPatternScore(patternScore);

        if (keyPuyos.size() == 0 && rensaResult.chains == 2) {
            sideChainMaxScore = std::max(sideChainMaxScore, rensaResult.score);
        }

        PuyoSet necessaryPuyos;
        necessaryPuyos.add(keyPuyos);
        necessaryPuyos.add(firePuyos);

        rensaEvaluator.evalRensaChainFeature(rensaResult, necessaryPuyos);
        rensaEvaluator.evalRensaGarbage(fieldAfterRensa);
        rensaEvaluator.evalFirePointTabooFeature(plan, trackResult);
        rensaEvaluator.evalRensaIgnitionHeightFeature(plan, trackResult, enemy.hasZenkeshi);
        rensaEvaluator.evalRensaConnectionFeature(fieldAfterRensa);

        rensaEvaluator.evalComplementationBias(keyPuyos, firePuyos);
        rensaEvaluator.evalRensaStrategy(plan, rensaResult, keyPuyos, firePuyos, currentFrameId, me, enemy);

        // TODO(mayah): need to set a better mode here.
        rensaScoreCollector->setMode(mode);

        if (rensaScoreCollector->score() > maxRensaScore) {
            maxRensaScore = rensaScoreCollector->score();
            maxRensaScoreCollector = move(rensaScoreCollector);
            maxRensaKeyPuyos = keyPuyos;
            maxRensaFirePuyos = firePuyos;
            maxScoreChains = rensaResult.chains;
        }

        const double rensaScore = rensaResult.score;
        double possibility = TsumoPossibility::possibility(necessaryPuyos, std::max(0, numReachableSpace));
        // possibility = std::min(1.0, possibility + 0.15);

        const double virtualRensaScore = rensaScore * possibility;
        rensaEvaluator.evalRensaScore(rensaScore, virtualRensaScore);

        if (maxVirtualRensaResultScore < virtualRensaScore) {
            maxVirtualRensaResultScore = virtualRensaScore;
        }

        int n = TsumoPossibility::necessaryPuyos(necessaryPuyos, 0.5);
        if (n <= 5 && fastChainMaxScore > rensaScore) {
            fastChainMaxScore = rensaScore;
        }
    };

    auto callback = [&](const CoreField& fieldAfterRensa, const RensaResult& rensaResult,
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                        const RensaChainTrackResult& trackResult, const string& patternName, double patternScore) {
        evalCallback(fieldBeforeRensa, fieldAfterRensa, rensaResult, keyPuyos, firePuyos, patternScore, patternName, trackResult);
    };
    PatternRensaDetector detector(patternBook(), fieldBeforeRensa, callback);
    detector.iteratePossibleRensas(preEvalResult.matchablePatternIds(), maxIteration);

    if (sideChainMaxScore >= scoreForOjama(21)) {
        sc_->addScore(HOLDING_SIDE_CHAIN_LARGE, 1);
    } else if (sideChainMaxScore >= scoreForOjama(15)) {
        sc_->addScore(HOLDING_SIDE_CHAIN_MEDIUM, 1);
    } else if (sideChainMaxScore >= scoreForOjama(12)) {
        sc_->addScore(HOLDING_SIDE_CHAIN_SMALL, 1);
    }

    if (fastChainMaxScore >= scoreForOjama(30)) {
        sc_->addScore(HOLDING_FAST_CHAIN_LARGE, 1);
    } else if (fastChainMaxScore >= scoreForOjama(18)) {
        sc_->addScore(HOLDING_FAST_CHAIN_MEDIUM, 1);
    }

    int rensaKind = 0;
    for (int i = maxScoreChains; i < 20; ++i)
        rensaKind += rensaCounts[i];
    sc_->addScore(RENSA_KIND, rensaKind);

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
