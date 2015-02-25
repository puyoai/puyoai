#include "ai.h"

#include <algorithm>
#include <climits>
#include <tuple>
#include <vector>

#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_detector.h"
#include "core/core_field.h"
#include "core/frame_request.h"
#include "core/rensa_result.h"

#include "evaluator.h"

munetoshi::grade inner_product(
        const munetoshi::grade* vect1,
        const munetoshi::grade* vect2,
        int n);

static const munetoshi::grade GRADE_WEIGHT_GROW[munetoshi::NUM_EVALUATOR_TYPES] = {
        10, // CHAIN_LENGTH
        -2, // NUM_REQUIRED_PUYO
        -5, // DEATH_RATIO
        -0.5, // TEAR
        -1, // VALLEY_SHAPE
        40, // TURNOVER_SHAPE
};

munetoshi::AI::AI(int argc, char* argv[]) :
        ::AI(argc, argv, "munetoshi") {
    strategy = GROW;
}

void munetoshi::AI::onGameWillBegin(const FrameRequest& /*frame*/) {
    strategy = GROW;
}

DropDecision munetoshi::AI::think(
        int frame_id,
        const CoreField& field,
        const KumipuyoSeq& seq,
        const PlayerState&,
        const PlayerState&,
        bool) const {
    return think_internal(frame_id, field, seq);
}

DropDecision munetoshi::AI::think_internal(
        int /*frame_id*/,
        const CoreField& field,
        const KumipuyoSeq& seq) const {

    Decision best_chain_decision;
    Decision best_fire_decision;
    grade best_chain_grade = GRADE_MIN;
    grade best_fire_grade = GRADE_MIN;
    grade previous_chain_grade = evaluate(field, nullptr);
    auto dicisionMaker = [&](const RefPlan& plan) {
        grade fire_grade = plan.rensaResult().score;
        grade chain_grade = evaluate(plan.field(), &plan);

        if (best_fire_grade < fire_grade) {
            best_fire_grade = fire_grade;
            best_fire_decision = plan.decisions().front();
        }

        if (best_chain_grade < chain_grade) {
            best_chain_grade = chain_grade;
            best_chain_decision = plan.decisions().front();
        }
    };

    Plan::iterateAvailablePlans(field, seq, 2, dicisionMaker);
    return best_chain_grade < previous_chain_grade
            || (strategy == FIRE && best_fire_grade > 1000) ?
            DropDecision(best_fire_decision, "munetoshi: FIRE") :
            DropDecision(best_chain_decision, "munetoshi: GROW");
}

void munetoshi::AI::onEnemyGrounded(const FrameRequest& frame) {
    CoreField field(frame.enemyPlayerFrameRequest().field);
    field.forceDrop();

    if (field.simulate().chains > 1) {
        strategy = FIRE;
    }
}

int munetoshi::AI::evaluate(const CoreField& core_field, const RefPlan *plan_ptr) const {
    grade optimal_grade = GRADE_MIN;
    auto callback = [&](
            const CoreField& /*field_after_chain*/,
            const RensaResult& rensa_result,
            const ColumnPuyoList& key_puyos,
            const ColumnPuyoList& fire_puyos,
            const RensaVanishingPositionResult& position_result) {

        grade grade_vect[NUM_EVALUATOR_TYPES] = {};
        EvaluationElements e = {
                plan_ptr,
                core_field,
                rensa_result,
                key_puyos,
                fire_puyos,
                position_result,
        };

        auto grade_vect_setter = [&](EVALUATOR_TYPES type, float result) {
            grade_vect[static_cast<size_t>(type)] = result;
        };

        Evaluators<
        EVALUATOR_TYPES::CHAIN_LENGTH,
        EVALUATOR_TYPES::NUM_REQUIRED_PUYO,
        EVALUATOR_TYPES::DEATH_RATIO,
        EVALUATOR_TYPES::TEAR,
        EVALUATOR_TYPES::VALLEY_SHAPE,
        EVALUATOR_TYPES::TURNOVER_SHAPE,
        EVALUATOR_TYPES::_NIL_TYPE>
        ::evaluate_all(e, grade_vect_setter);

        optimal_grade = std::max(
                inner_product(grade_vect, GRADE_WEIGHT_GROW, NUM_EVALUATOR_TYPES),
                optimal_grade);
    };

    RensaDetector::iteratePossibleRensasWithVanishingPositionTracking(core_field, 1,
            RensaDetectorStrategy::defaultFloatStrategy(), callback);
    return optimal_grade;
}

munetoshi::grade inner_product(
        const munetoshi::grade* vect1,
        const munetoshi::grade* vect2,
        int n) {
    munetoshi::grade sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += vect1[i] * vect2[i];
    }
    return sum;
}
