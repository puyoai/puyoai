#include "evaluator.h"

#include <algorithm>
#include <array>

#include "core/plan/plan.h"
#include "core/column_puyo_list.h"
#include "core/rensa_result.h"
#include "core/rensa_tracker/rensa_vanishing_position_tracker.h"

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
    std::min(1,2);
    std::min(1,2+1);
    for (int col = 1; col <= FieldConstant::WIDTH; ++col) {
        int valley_depth = std::min(
                (col == 1 ? FieldConstant::MAP_HEIGHT : e->field.height(col - 1)) - e->field.height(col),
                (col == FieldConstant::WIDTH ? FieldConstant::MAP_HEIGHT : e->field.height(col + 1)) - e->field.height(col));
        valley_depth *= col == 1 || col == FieldConstant::WIDTH ? 10 : 1;
        if (valley_depth >= 2) {
            valley_grade += valley_depth;
        }
    }

    for (int col = 2; col < FieldConstant::WIDTH; ++col) {
        int mountain_hight = std::max(
                e->field.height(col) - e->field.height(col - 1),
                e->field.height(col) - e->field.height(col + 1));
        if (mountain_hight >= 3) {
            valley_grade += mountain_hight;
        }
    }
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
#if 1
    // TODO(mayah); ColumnPuyoList now does not provide iterator.
    // I believe this is the code that you want. If not, please rewrite this.
    return e->puyos_to_complement.size();
#else
    // original code is here.
    int required_puyos = 0;
    auto adder = [&](const ColumnPuyo& cp) {required_puyos += cp.x;};
    std::for_each(e->puyos_to_complement.begin(), e->puyos_to_complement.end(), adder);
    return required_puyos;
#endif
}

template<>
grade Evaluator<EVALUATOR_TYPES::TURNOVER_SHAPE, PossibleChainResult>::EVALUATE(
        PossibleChainResult* e) {
    for (int nth_chain = 1;
            nth_chain <= (int) e->position_result.size();
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
            e->turnover_bottom = nth_chain;
            return 1;
        }
    }
    return 0;
}

template<>
grade Evaluator<EVALUATOR_TYPES::TURNOVER_HEAD, PossibleChainResult>::EVALUATE(
        PossibleChainResult* e) {
    if (e->turnover_bottom == 0) {
        return 0;
    }
    return e->rensa_result.chains - e->turnover_bottom;
}

} // namespace munetoshi
