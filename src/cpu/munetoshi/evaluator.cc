#include "evaluator.h"

#include <algorithm>
#include <array>

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
    constexpr size_t X_IDX = 0;
    constexpr size_t Y_IDX = 1;

    for (size_t nth_chain = 1;
            nth_chain <= static_cast<size_t>(e->position_result.size());
            ++nth_chain) {

        int has_left_side = 0;
        int has_left_center = 0;
        bool has_left_penalty = false;

        int has_right_side = 0;
        int has_right_center = 0;
        bool has_right_penalty = false;

        for (auto base_puyo : e->position_result.getReferenceBasePuyosAt(nth_chain)) {
            if (base_puyo.x <= 2 && base_puyo.y == 1) {
                has_left_side = std::max(has_left_side, 3 - base_puyo.x);
            } else if (base_puyo.x == 3 && base_puyo.y <= 2) {
                has_left_center = std::max(has_left_center, base_puyo.y);
            } else if (base_puyo.x == 4 && base_puyo.y == 1) {
                has_left_penalty = true;
            } else if (base_puyo.x == 1 && base_puyo.y == 2) {
                has_left_penalty = true;
            } else if (base_puyo.x >= 5 && base_puyo.y == 1) {
                has_right_side = std::max(has_right_side, base_puyo.x - 4);
            } else if (base_puyo.x == 4 && base_puyo.y <= 2) {
                has_right_center = std::max(has_right_center, base_puyo.y);
            } else if (base_puyo.x == 3 && base_puyo.y == 1) {
                has_right_penalty = true;
            } else if (base_puyo.x == 6 && base_puyo.y == 2) {
                has_right_penalty = true;
            }
        }

        TurnoverType possible_turnover =
                has_left_side != 0 && has_left_center != 0 && !has_left_penalty ?
                        TurnoverType::LEFT
                        : (has_right_side != 0 && has_right_center != 0 && !has_right_penalty ?
                                TurnoverType::RIGHT
                                : TurnoverType::NIL);

        if (possible_turnover == TurnoverType::NIL) {
            continue;
        }

        std::array<float, 2> nth_weight =
                e->position_result.getWeightedCenterAfterFall(nth_chain);
        int possible_prev_chain = 1;
        int possible_next_chain = 1;

        if (nth_chain != 1) {
            std::array<float, 2> previous_weight =
                    e->position_result.getWeightedCenterAfterFall(nth_chain - 1);
            if (nth_weight[Y_IDX] <= previous_weight[Y_IDX]
                    && (possible_turnover == TurnoverType::LEFT ?
                            (nth_weight[X_IDX] > previous_weight[X_IDX])
                            : (nth_weight[X_IDX] < previous_weight[X_IDX]))) {
                possible_prev_chain = 2;
            } else {
                possible_prev_chain = 0;
            }
        }
        if (nth_chain != static_cast<size_t>(e->position_result.size())) {
            std::array<float, 2> next_weight =
                    e->position_result.getWeightedCenterAfterFall(nth_chain + 1);
            if (possible_turnover == TurnoverType::LEFT ?
                    (nth_weight[X_IDX] < next_weight[X_IDX])
                    : (nth_weight[X_IDX] > next_weight[X_IDX])) {
                possible_next_chain = 2;
            } else {
                possible_next_chain = 0;
            }
        }

        if (possible_prev_chain != 0 && possible_next_chain != 0) {
            e->turnover_type = possible_turnover;
            e->turnover_bottom = nth_chain;
            return possible_turnover == TurnoverType::LEFT ?
                    has_left_side + has_left_center + possible_prev_chain + possible_next_chain :
                    has_right_side + has_right_center + possible_prev_chain + possible_next_chain;
        }
        return 0;
    }
    return 0;
}

} // namespace munetoshi
