#pragma once

#include <cstdint>

#include "constants.h"

struct PlayerState;
struct RensaResult;

class CoreField;
class ColumnPuyoList;
class RefPlan;
class RensaVanishingPositionResult;

namespace munetoshi {

typedef uint8_t chain_idx;

enum class TurnoverType {
    NIL,
    LEFT,
    RIGHT,
};

struct PlanResult {
    const CoreField& field;
    const PlayerState& my_state;
    const PlayerState& opponent_state;

    const RefPlan* plan_ptr;
};

struct PossibleChainResult : public PlanResult {
    const RensaResult& rensa_result;
    const ColumnPuyoList& key_puyos;
    const ColumnPuyoList& fire_puyos;
    const RensaVanishingPositionResult& position_result;

    // These are non const because they are set by an evaluator.
    chain_idx turnover_bottom;
    chain_idx turnover_top;
    chain_idx base_end;
    TurnoverType turnover_type;


    PossibleChainResult(
            const PlanResult& plan_result,
            const RensaResult& rensa_result,
            const ColumnPuyoList& key_puyos,
            const ColumnPuyoList& fire_puyos,
            const RensaVanishingPositionResult& position_result) :
    PlanResult(plan_result),
    rensa_result(rensa_result),
    key_puyos(key_puyos),
    fire_puyos(fire_puyos),
    position_result(position_result),
    turnover_bottom(0),
    turnover_top(0),
    base_end(0),
    turnover_type(TurnoverType::NIL){
    }
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
grade Evaluator<EVALUATOR_TYPES::DEATH_RATIO, PlanResult>::EVALUATE(
            PlanResult* e);

template<>
grade Evaluator<EVALUATOR_TYPES::TEAR, PlanResult>::EVALUATE(
            PlanResult* e);

template<>
grade Evaluator<EVALUATOR_TYPES::VALLEY_SHAPE, PlanResult>::EVALUATE(
            PlanResult* e);

template<>
grade Evaluator<EVALUATOR_TYPES::CHAIN_LENGTH, PossibleChainResult>::EVALUATE(
            PossibleChainResult* e);

template<>
grade Evaluator<EVALUATOR_TYPES::NUM_REQUIRED_PUYO, PossibleChainResult>::EVALUATE(
            PossibleChainResult* e);

template<>
grade Evaluator<EVALUATOR_TYPES::TURNOVER_SHAPE, PossibleChainResult>::EVALUATE(
            PossibleChainResult* e);
} // namespace munetoshi
