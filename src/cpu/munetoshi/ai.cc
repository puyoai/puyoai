#include "ai.h"

#include <algorithm>
#include <climits>
#include <tuple>
#include <vector>

#include "core/algorithm/column_puyo_list.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_detector.h"
#include "core/core_field.h"
#include "core/frame_request.h"
#include "core/rensa_result.h"

double inner_product(const double* vect1, const double* vect2, int n);

static const double GRADE_WEIGHT_GROW[munetoshi::AI::GRADE_NUM] = {
		10,
		-2,
		-10,
		-1,
		-4,
		-4,
		-4,
		-4,
		-4,
		-4,
		50,
};

munetoshi::AI::AI(int argc, char* argv[]) : ::AI(argc, argv, "munetoshi") {
  strategy = GROW;
}

void munetoshi::AI::onGameWillBegin(const FrameRequest& frame) {
  UNUSED_VARIABLE(frame);
  strategy = GROW;
}

DropDecision munetoshi::AI::think(int frame_id, const CoreField& field,
                                  const KumipuyoSeq& seq,
                                  const PlayerState&,
                                  const PlayerState&,
                                  bool) const {
  return think_internal(frame_id, field, seq);
}

DropDecision munetoshi::AI::think_internal(int frame_id,
                                           const CoreField& field,
                                           const KumipuyoSeq& seq) const {
  UNUSED_VARIABLE(frame_id);

  Decision best_chain_decision;
  Decision best_fire_decision;
  int best_chain_grade = -1;
  int best_fire_grade = -1;
  int previous_chain_grade = evaluate(field, nullptr);
  auto dicisionMaker = [&](const RefPlan& plan) {
    int fire_grade = plan.rensaResult().score;
    int chain_grade = evaluate(plan.field(), &plan);

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
    || (strategy == FIRE && best_fire_grade > 1000)
             ? DropDecision(best_fire_decision, "munetoshi: FIRE")
             : DropDecision(best_chain_decision, "munetoshi: GROW");
}

void munetoshi::AI::onEnemyGrounded(const FrameRequest& frame) {
  CoreField field(frame.enemyPlayerFrameRequest().field);
  field.forceDrop();

  if (field.simulate().chains > 1) {
    strategy = FIRE;
  }
}

int munetoshi::AI::evaluate(const CoreField& field, const RefPlan *plan) const {
  int grade = INT_MIN;
  int required_puyos;
  auto adder = [&](const ColumnPuyo& cp) { required_puyos += cp.x; };
  auto callback = [&](const CoreField&, const RensaResult& rensa_result,
                      const ColumnPuyoList& key_puyos, const ColumnPuyoList& fire_puyos,
                      const RensaVanishingPositionResult& position_result) {
	int turn_down_chain = 0;
	for (int nth_chain = 1;
	    nth_chain <= (int) position_result.size() && turn_down_chain == 0;
	    ++nth_chain) {
	  bool has_left_side = false;
	  bool has_left_center = false;
	  bool has_right_side = false;
	  bool has_right_center = false;
	  for (auto base_puyo : position_result.getReferenceBasePuyosAt(nth_chain)) {
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
	required_puyos = 0;
    std::for_each(key_puyos.begin(), key_puyos.end(), adder);
    std::for_each(fire_puyos.begin(), fire_puyos.end(), adder);
    double grade_vect[GRADE_NUM];
    grade_vect[CHAIN_LENGTH] = rensa_result.chains;
    grade_vect[NUM_REQUIRED_PUYO] = required_puyos;
    grade_vect[DEATH_RATIO] = std::max(field.height(3) - 9, 0);
    grade_vect[TEAR] = plan != nullptr && plan->numChigiri() > 0
    		? 1 : 0;
    grade_vect[GRACE_VALLEY_2_1] =
    		std::max(field.height(2) - field.height(1), 0);
    grade_vect[GRACE_VALLEY_3_2] =
    		std::max(field.height(3) - field.height(2), 0);
    grade_vect[GRACE_VALLEY_3_4] =
    		std::max(field.height(3) - field.height(4), 0);
    grade_vect[GRACE_VALLEY_4_5] =
    		std::max(field.height(4) - field.height(5), 0);
    grade_vect[GRACE_VALLEY_5_6] =
    		std::max(field.height(5) - field.height(6), 0);
    grade_vect[GRACE_VALLEY_4_3_GT2] =
    		std::max(field.height(4) - field.height(3) - 2, 0);
    grade_vect[TURNDOWN] = turn_down_chain != 0 ? 1 : 0; 
    grade = std::max(
    		(int) inner_product(grade_vect, GRADE_WEIGHT_GROW, GRADE_NUM),
			grade);
  };

  RensaDetector::iteratePossibleRensasWithVanishingPositionTracking(field, 1, RensaDetectorStrategy::defaultFloatStrategy(), callback);
  return std::max(grade, 0);
}

double inner_product(const double* vect1, const double* vect2, int n) {
	double sum = 0;
	for (int i = 0; i < n; ++i) {
		sum += vect1[i] * vect2[i];
	}
	return sum;
}
