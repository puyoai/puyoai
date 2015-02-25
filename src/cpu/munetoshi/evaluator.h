#pragma once

#include "constants.h"

struct PlayerState;
struct RensaResult;

class CoreField;
class ColumnPuyoList;
class RefPlan;
class RensaVanishingPositionResult;

namespace munetoshi {

struct EvaluationElements {
    const CoreField& field;
    const PlayerState& my_state;
    const PlayerState& opponent_state;
    const RensaResult& rensa_result;
    const ColumnPuyoList& key_puyos;
    const ColumnPuyoList& fire_puyos;
    const RensaVanishingPositionResult& position_result;

    const RefPlan* plan_ptr;
};

template<EVALUATOR_TYPES TYPE, typename PARAM>
class Evaluator {
public:
    static grade EVALUATE(PARAM* e);
private:
    Evaluator() = delete;
    Evaluator(const Evaluator&) = delete;
    Evaluator(Evaluator&&) = delete;
    Evaluator& operator =(const Evaluator&) = delete;
    Evaluator& operator =(Evaluator&&) = delete;
};

template<typename PARAM, EVALUATOR_TYPES... TYPES>
struct Evaluators;

template<typename PARAM, EVALUATOR_TYPES FIRST, EVALUATOR_TYPES... REST>
struct Evaluators<PARAM, FIRST, REST...> {
    template<typename FUNC>
    static void evaluate_all(
            PARAM* e,
            const FUNC& f) {
        f(FIRST, Evaluator<FIRST, PARAM>::EVALUATE(e));
        Evaluators<PARAM, REST...>::evaluate_all(e, f);
    }
};

template<typename PARAM>
struct Evaluators<PARAM, EVALUATOR_TYPES::_NIL_TYPE> {
    template<typename FUNC>
    static void evaluate_all(
            PARAM*,
            const FUNC&) {
        // Do nothing.
    }
};

template<>
grade Evaluator<EVALUATOR_TYPES::CHAIN_LENGTH, EvaluationElements>::EVALUATE(
            EvaluationElements* e);

template<>
grade Evaluator<EVALUATOR_TYPES::NUM_REQUIRED_PUYO, EvaluationElements>::EVALUATE(
            EvaluationElements* e);

template<>
grade Evaluator<EVALUATOR_TYPES::DEATH_RATIO, EvaluationElements>::EVALUATE(
            EvaluationElements* e);

template<>
grade Evaluator<EVALUATOR_TYPES::TEAR, EvaluationElements>::EVALUATE(
            EvaluationElements* e);

template<>
grade Evaluator<EVALUATOR_TYPES::VALLEY_SHAPE, EvaluationElements>::EVALUATE(
            EvaluationElements* e);

template<>
grade Evaluator<EVALUATOR_TYPES::TURNOVER_SHAPE, EvaluationElements>::EVALUATE(
            EvaluationElements* e);
} // namespace munetoshi
