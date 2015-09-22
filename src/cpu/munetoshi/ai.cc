#include "ai.h"

#include <algorithm>
#include <climits>
#include <string>
#include <tuple>
#include <vector>

#include "base/strings.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/core_field.h"
#include "core/frame_request.h"
#include "core/rensa_result.h"
#include "core/rensa_tracker/rensa_vanishing_position_tracker.h"

#include "evaluator.h"

namespace {
template<int N>
munetoshi::grade inner_product(
        const std::array<munetoshi::grade, N>& vect1,
        const std::array<munetoshi::grade, N>& vect2);

constexpr std::array<munetoshi::grade, munetoshi::NUM_EVALUATOR_TYPES> GRADE_WEIGHT_GROW = {{
        -10, // DEATH_RATIO
        -1, // TEAR
        -2, // VALLEY_SHAPE
        20, // CHAIN_LENGTH

        -4, // NUM_REQUIRED_PUYO
        10, // TURNOVER_SHAPE
        1, // TURNOVER_HEAD
}};
} // namespace

munetoshi::AI::AI(int argc, char* argv[]) :
        ::AI(argc, argv, "minim") {
    strategy = GROW;
}

void munetoshi::AI::onGameWillBegin(const FrameRequest& /*frame*/) {
    strategy = GROW;
}

DropDecision munetoshi::AI::think(
        int /*frame_id*/,
        const CoreField& field,
        const KumipuyoSeq& seq,
        const PlayerState& my_state,
        const PlayerState& opponent_state,
        bool) const {
    return think_internal(field, seq, my_state, opponent_state);
}

DropDecision munetoshi::AI::think_internal(
        const CoreField& field,
        const KumipuyoSeq& seq,
        const PlayerState& my_state,
        const PlayerState& opponent_state) const {

    std::string message;
    Decision best_chain_decision;
    Decision best_fire_decision;
    grade best_chain_grade = GRADE_MIN;
    grade best_fire_grade = GRADE_MIN;
    grade previous_chain_grade = evaluate(
            field,
            my_state,
            opponent_state);
    auto dicisionMaker = [&](const RefPlan& plan) {
        grade fire_grade = plan.rensaResult().score;
        grade chain_grade = evaluate(
                plan.field(),
                my_state,
                opponent_state,
                &plan);

        if (best_fire_grade < fire_grade) {
            best_fire_grade = fire_grade;
            best_fire_decision = plan.decisions().front();
        }

        if (best_chain_grade < chain_grade) {
            best_chain_grade = chain_grade;
            best_chain_decision = plan.decisions().front();
            message = AI_NAME
                    + (strategy == FIRE ? "ヽ(｀Д´#)ﾉ" : "ヽ(´ー｀)ノ" )
                    + std::to_string(best_chain_grade);
        }
    };

    Plan::iterateAvailablePlans(field, seq, 2 /*max_depth*/, dicisionMaker);
    return best_chain_grade < previous_chain_grade
            || (strategy == FIRE && best_fire_grade > 1000) ?
            DropDecision(best_fire_decision, message):
            DropDecision(best_chain_decision, message);
}

void munetoshi::AI::onGroundedForEnemy(const FrameRequest& frame) {
    CoreField field(CoreField::fromPlainFieldWithDrop(frame.enemyPlayerFrameRequest().field));

    if (field.simulate().chains > 1) {
        strategy = FIRE;
    }
}

munetoshi::grade munetoshi::AI::evaluate(
        const CoreField& core_field,
        const PlayerState& my_state,
        const PlayerState& opponent_state,
        const RefPlan *plan_ptr) const {

    grade optimal_grade = GRADE_MIN;

    PlanResult plan_result = {
            core_field,
            my_state,
            opponent_state,
            plan_ptr,
    };

    std::array<grade, NUM_EVALUATOR_TYPES> plan_grade_vect {{}};
    auto plan_grade_vect_setter = [&](
            EVALUATOR_TYPES type,
            grade result) {
        plan_grade_vect[static_cast<size_t>(type)] = result;
    };

    Evaluators<
    PlanResult,
    EVALUATOR_TYPES::DEATH_RATIO,
    EVALUATOR_TYPES::TEAR,
    EVALUATOR_TYPES::VALLEY_SHAPE,
    EVALUATOR_TYPES::_NIL_TYPE>
    ::evaluate_all(&plan_result, plan_grade_vect_setter);

    auto callback = [&](CoreField&& complemented_field,
                        const ColumnPuyoList& puyos_to_complement) -> RensaResult {
        RensaVanishingPositionTracker tracker;
        const RensaResult rensa_result = complemented_field.simulate(&tracker);
        const RensaVanishingPositionResult& position_result = tracker.result();

        PossibleChainResult possible_chain_result = {
                plan_result,
                rensa_result,
                puyos_to_complement,
                position_result,
        };

        std::array<grade, NUM_EVALUATOR_TYPES> possible_chain_grade_vect = plan_grade_vect;;
        auto possible_chain_grade_vect_setter = [&](
                EVALUATOR_TYPES type,
                grade result) {
            possible_chain_grade_vect[static_cast<size_t>(type)] = result;
        };

        Evaluators<
        PossibleChainResult,
        EVALUATOR_TYPES::CHAIN_LENGTH,
        EVALUATOR_TYPES::NUM_REQUIRED_PUYO,
        EVALUATOR_TYPES::TURNOVER_SHAPE,
        EVALUATOR_TYPES::TURNOVER_HEAD,
        EVALUATOR_TYPES::_NIL_TYPE>
        ::evaluate_all(&possible_chain_result, possible_chain_grade_vect_setter);

        optimal_grade = std::max(
                inner_product<NUM_EVALUATOR_TYPES>(possible_chain_grade_vect, GRADE_WEIGHT_GROW),
                optimal_grade);

        return rensa_result;
    };

    RensaDetector::detectIteratively(core_field, RensaDetectorStrategy::defaultFloatStrategy(), 2, callback);
    return optimal_grade;
}

namespace {
template<int N>
munetoshi::grade inner_product(
        const std::array<munetoshi::grade, N>& vect1,
        const std::array<munetoshi::grade, N>& vect2) {
    munetoshi::grade sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += vect1[i] * vect2[i];
    }
    return sum;
}
} // namespace
