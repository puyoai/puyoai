#include "rensa_evaluator.h"

#include "base/base.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/plan/plan.h"
#include "core/player_state.h"
#include "core/probability/column_puyo_list_probability.h"
#include "core/probability/puyo_set.h"

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::eval(const CoreField& complementedField,
                                          const CoreField& fieldBeforeRensa,
                                          const CoreField& fieldAfterRensa,
                                          const RensaResult& rensaResult,
                                          const ColumnPuyoList& puyosToComplement,
                                          double patternScore,
                                          double virtualRensaScore)
{
    FieldBits ignitionPuyoBits = complementedField.ignitionPuyoBits();

    evalRensaRidgeHeight(complementedField);
    evalRensaValleyDepth(complementedField);
    evalRensaFieldUShape(complementedField);
    evalRensaIgnitionHeightFeature(complementedField, ignitionPuyoBits);
    evalRensaChainFeature(rensaResult, puyosToComplement);
    evalRensaGarbage(fieldAfterRensa);
    evalPatternScore(puyosToComplement, patternScore, rensaResult.chains);
    evalFirePointTabooFeature(fieldBeforeRensa, ignitionPuyoBits); // fieldBeforeRensa is correct.
    evalRensaConnectionFeature(fieldAfterRensa);
    evalComplementationBias(puyosToComplement);
    evalRensaScore(rensaResult.score, virtualRensaScore);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaStrategy(const RefPlan& plan,
                                                       const RensaResult& rensaResult,
                                                       const ColumnPuyoList& cpl,
                                                       int currentFrameId,
                                                       const PlayerState& me,
                                                       const PlayerState& enemy)
{
    UNUSED_VARIABLE(currentFrameId);
    UNUSED_VARIABLE(me);

    if (plan.field().countPuyos() >= 36 && plan.score() >= scoreForOjama(15) &&
        plan.chains() <= 3 && rensaResult.chains >= 7 &&
        cpl.size() <= 3 && !enemy.isRensaOngoing()) {
        sc_->addScore(STRATEGY_SAISOKU, 1);
    }
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaChainFeature(const RensaResult& rensaResult,
                                                           const ColumnPuyoList& cplToComplement)
{
    sc_->addScore(MAX_CHAINS, rensaResult.chains, 1);

    double numKumipuyos = ColumnPuyoListProbability::instanceSlow()->necessaryKumipuyos(cplToComplement);
    sc_->addScore(NECESSARY_PUYOS_LINEAR, numKumipuyos);
    sc_->addScore(NECESSARY_PUYOS_SQUARE, numKumipuyos * numKumipuyos);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalFirePointTabooFeature(const CoreField& field,
                                                               const FieldBits& ignitionPuyoBits)
{
    // A_A is taboo (unless x == 1 or x == 4)
    //
    //      A
    // A_AA A_A are also taboo (including x == 1 and x == 4).
    for (int x = 1; x <= 4; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (!ignitionPuyoBits.get(x, y) || !ignitionPuyoBits.get(x + 1, y) || !ignitionPuyoBits.get(x + 2, y))
                continue;

            if (!field.isEmpty(x + 1, y))
                continue;
            if (!field.isNormalColor(x, y))
                continue;
            if (field.color(x, y) != field.color(x + 2, y))
                continue;

            if (x != 1 && x != 4) {
                sc_->addScore(FIRE_POINT_TABOO, 1);
                return;
            }

            if (field.color(x, y) == field.color(x, y + 1) || field.color(x, y) == field.color(x + 2, y + 1) ||
                field.color(x, y) == field.color(x - 1, y) || field.color(x, y) == field.color(x + 3, y)) {
                sc_->addScore(FIRE_POINT_TABOO, 1);
                return;
            }
        }
    }
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaIgnitionHeightFeature(const CoreField& complementedField,
                                                                    const FieldBits& ignitionPuyoBits)
{
    FieldBits ignition = ignitionPuyoBits & complementedField.bitField().normalColorBits();
    int height = ignition.highestHeight();
    if (height >= 0)
        sc_->addScore(IGNITION_HEIGHT, height, 1);

    int higherPuyoLinear = 0;
    int higherPuyoSquare = 0;
    for (int x = 1; x <= 6; ++x) {
        if (complementedField.height(x) > height) {
            int d = complementedField.height(x) - height;
            higherPuyoLinear += d;
            higherPuyoSquare += d * d;
        }
    }
    sc_->addScore(HIGHER_PUYO_THAN_IGNITION_LINEAR, higherPuyoLinear);
    sc_->addScore(HIGHER_PUYO_THAN_IGNITION_SQUARE, higherPuyoSquare);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaConnectionFeature(const CoreField& fieldAfterDrop)
{
    int count2, count3;
    fieldAfterDrop.countConnection(&count2, &count3);
    sc_->addScore(CONNECTION_AFTER_DROP_2, count2);
    sc_->addScore(CONNECTION_AFTER_DROP_3, count3);
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
    for (int x = 1; x <= 6; ++x) {
        sc_->addScore(RENSA_RIDGE_HEIGHT, field.ridgeHeight(x), 1);
    }
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaValleyDepth(const CoreField& field)
{
    for (int x = 1; x <= 6; ++x) {
        if (x == 1 || x == 6)
            sc_->addScore(RENSA_VALLEY_DEPTH_EDGE, field.valleyDepth(x), 1);
        else
            sc_->addScore(RENSA_VALLEY_DEPTH, field.valleyDepth(x), 1);
    }
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalRensaFieldUShape(const CoreField& field)
{
    static const int DIFF[FieldConstant::MAP_WIDTH] = {
        0, -3, 0, 1, 1, 0, -3, 0,
    };

    static const double FIELD_USHAPE_HEIGHT_COEF[15] = {
        0.0, 0.1, 0.1, 0.3, 0.3,
        0.5, 0.5, 0.7, 0.7, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
    };

    double average = 0;
    for (int x = 1; x <= 6; ++x)
        average += (field.height(x) + DIFF[x]);
    average /= 6;

    double coef = FIELD_USHAPE_HEIGHT_COEF[static_cast<int>(average)];

    double linearValue = 0;
    double squareValue = 0;

    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        int h = field.height(x) + DIFF[x];
        linearValue += std::abs(h - average) * coef;
        squareValue += (h - average) * (h - average) * coef;
    }

    sc_->addScore(RENSA_FIELD_USHAPE_LINEAR, linearValue);
    sc_->addScore(RENSA_FIELD_USHAPE_SQUARE, squareValue);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalPatternScore(const ColumnPuyoList& cpl, double patternScore, int chains)
{
    if (chains > 0)
        sc_->addScore(PATTERN_BOOK_DIV_RENSA, patternScore / chains);
    sc_->addScore(PATTERN_BOOK, patternScore);

    int numPlaceHolders = 0;
    for (int x = 1; x <= 6; ++x) {
        int h = cpl.sizeOn(x);
        for (int i = 0; i < h; ++i) {
            if (!isNormalColor(cpl.get(x, i)))
                ++numPlaceHolders;
        }
    }

    sc_->addScore(NUM_PLACE_HOLDERS, numPlaceHolders);
}

template<typename ScoreCollector>
void RensaEvaluator<ScoreCollector>::evalComplementationBias(const ColumnPuyoList& cpl)
{
    for (int x = 1; x <= 6; ++x) {
        PuyoSet ps;
        int h = cpl.sizeOn(x);
        for (int i = 0; i < h; ++i)
            ps.add(cpl.get(x, i));

        if (ps.count() < 3)
            continue;
        if (ps.count() >= 4)
            sc_->addScore(COMPLEMENTATION_BIAS_MUCH, 1);
        if (ps.red() >= 3 || ps.blue() >= 3 || ps.yellow() >= 3 || ps.green() >= 3) {
            EvaluationRensaFeatureKey key = (x == 1 || x == 6) ? COMPLEMENTATION_BIAS_EDGE : COMPLEMENTATION_BIAS;
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

template class RensaEvaluator<FeatureRensaScoreCollector>;
template class RensaEvaluator<SimpleRensaScoreCollector>;
