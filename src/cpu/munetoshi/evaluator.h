#pragma once

#include "constants.h"

struct RensaResult;
class RefPlan;
class CoreField;
class ColumnPuyoList;
class RensaVanishingPositionResult;

namespace munetoshi {

struct EvaluationElements {
    const RefPlan* plan_ptr;
    const CoreField& field;
    const RensaResult& rensa_result;
    const ColumnPuyoList& key_puyos;
    const ColumnPuyoList& fire_puyos;
    const RensaVanishingPositionResult& position_result;
};

template<EVALUATOR_TYPES E>
class Evaluator {
public:
    static grade EVALUATE(const EvaluationElements& e);
private:
    Evaluator() = delete;
    Evaluator(const Evaluator&) = delete;
    Evaluator(Evaluator&&) = delete;
    Evaluator& operator =(const Evaluator&) = delete;
    Evaluator& operator =(Evaluator&&) = delete;
};

template<EVALUATOR_TYPES... TYPES>
struct Evaluators;

template<EVALUATOR_TYPES FIRST, EVALUATOR_TYPES... REST>
struct Evaluators<FIRST, REST...> {
    template<typename FUNC>
    static void evaluate_all(
            const EvaluationElements& e,
            const FUNC& f) {
        f(FIRST, Evaluator<FIRST>::EVALUATE(e));
        Evaluators<REST...>::evaluate_all(e, f);
    }
};

template<>
struct Evaluators<EVALUATOR_TYPES::_NIL_TYPE> {
    template<typename FUNC>
    static void evaluate_all(
            const EvaluationElements&,
            const FUNC&) {
        // Do nothing.
    }
};

template<>
grade Evaluator<EVALUATOR_TYPES::CHAIN_LENGTH>::EVALUATE(
            const EvaluationElements& e);

template<>
grade Evaluator<EVALUATOR_TYPES::NUM_REQUIRED_PUYO>::EVALUATE(
            const EvaluationElements& e);

template<>
grade Evaluator<EVALUATOR_TYPES::DEATH_RATIO>::EVALUATE(
            const EvaluationElements& e);

template<>
grade Evaluator<EVALUATOR_TYPES::TEAR>::EVALUATE(
            const EvaluationElements& e);

template<>
grade Evaluator<EVALUATOR_TYPES::VALLEY_SHAPE>::EVALUATE(
            const EvaluationElements& e);

template<>
grade Evaluator<EVALUATOR_TYPES::TURNOVER_SHAPE>::EVALUATE(
            const EvaluationElements& e);
} // namespace munetoshi
