#include "evaluator.h"

#include <algorithm>

#include "core/algorithm/column_puyo_list.h"
#include "core/algorithm/plan.h"
#include "core/rensa_result.h"

namespace munetoshi {

template<>
grade Evaluator<EVALUATOR_TYPES::DEATH_RATIO, PlanResult>::EVALUATE(
        PlanResult* e) {
    return std::max(e->field.height(3) - 9, 0);
}

template<>
grade Evaluator<EVALUATOR_TYPES::TEAR, PlanResult>::EVALUATE(
            PlanResult* e) {
    return e->plan_ptr != nullptr ? e->plan_ptr->numChigiri() : 0;
}

template<>
grade Evaluator<EVALUATOR_TYPES::VALLEY_SHAPE, PlanResult>::EVALUATE(
            PlanResult* e) {
    grade valley_grade = 0;
    valley_grade += std::max(e->field.height(2) - e->field.height(1), 0) * 10;
    valley_grade += std::max(e->field.height(3) - e->field.height(2), 0);
    valley_grade += std::max(e->field.height(3) - e->field.height(4), 0);
    valley_grade += std::max(e->field.height(4) - e->field.height(5), 0);
    valley_grade += std::max(e->field.height(5) - e->field.height(6), 0) * 10;
    valley_grade += std::max(e->field.height(4) - e->field.height(3) - 2, 0);
    return valley_grade;
}


template<>
grade Evaluator<EVALUATOR_TYPES::CHAIN_LENGTH, PossibleChainResult>::EVALUATE(
        PossibleChainResult* e) {
    return e->rensa_result.chains;
}

template<>
grade Evaluator<EVALUATOR_TYPES::NUM_REQUIRED_PUYO, PossibleChainResult>::EVALUATE(
        PossibleChainResult* e) {
    int required_puyos = 0;
    auto adder = [&](const ColumnPuyo& cp) {required_puyos += cp.x;};
    std::for_each(e->key_puyos.begin(), e->key_puyos.end(), adder);
    std::for_each(e->fire_puyos.begin(), e->fire_puyos.end(), adder);
    return required_puyos;
}

template<>
grade Evaluator<EVALUATOR_TYPES::TURNOVER_SHAPE, PossibleChainResult>::EVALUATE(
        PossibleChainResult* e) {
    int turn_down_chain = 0;
    for (int nth_chain = 1;
            nth_chain <= (int) e->position_result.size() && turn_down_chain == 0;
            ++nth_chain) {
        bool has_left_side = false;
        bool has_left_center = false;
        bool has_right_side = false;
        bool has_right_center = false;
        for (auto base_puyo : e->position_result.getReferenceBasePuyosAt(nth_chain)) {
            if (base_puyo.x <= 2 && base_puyo.y == 1) {
                has_left_side = true;
            } else if (base_puyo.x == 3 && base_puyo.y <= 2) {
                has_left_center = true;
            } else if (base_puyo.x >= 5 && base_puyo.y == 1) {
                has_right_side = true;
            } else if (base_puyo.x == 4 && base_puyo.y <= 2) {
                has_right_center = true;
            }
        }
        if ((has_left_side && has_left_center) || (has_right_side && has_right_center)) {
            turn_down_chain = nth_chain;
        }
    }

    return turn_down_chain != 0 ? 1 : 0;
}

} // namespace munetoshi
