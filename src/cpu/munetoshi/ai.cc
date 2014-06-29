#include "ai.h"

#include <algorithm>
#include <tuple>
#include <vector>

#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_info.h"
#include "core/algorithm/rensa_detector.h"
#include "core/field/core_field.h"
#include "core/field/rensa_result.h"

munetoshi::AI::AI() : ::AI("munetoshi") { strategy = GROW; }

void munetoshi::AI::gameWillBegin(const FrameData& frame) {
  UNUSED_VARIABLE(frame);
  strategy = GROW;
}

DropDecision munetoshi::AI::think(int frame_id, const PlainField& field,
                                  const Kumipuyo& next1,
                                  const Kumipuyo& next2) {
  return think_internal(frame_id, field, next1, next2);
}

DropDecision munetoshi::AI::think_internal(int frame_id,
                                           const PlainField& field,
                                           const Kumipuyo& next1,
                                           const Kumipuyo& next2) {
  UNUSED_VARIABLE(frame_id);

  KumipuyoSeq seq{next1, next2};
  Decision best_chain_decision;
  Decision best_fire_decision;
  int best_chain_grade = -1;
  int best_fire_grade = -1;
  int previous_chain_grade = evaluate(field);
  auto dicisionMaker = [&](const RefPlan& plan) {
    int fire_grade = plan.rensaResult().score;
    int chain_grade = evaluate(plan.field());

    if (best_fire_grade < fire_grade) {
      best_fire_grade = fire_grade;
      best_fire_decision = plan.decisions().front();
    }

    if (best_chain_grade < chain_grade) {
      best_chain_grade = chain_grade;
      best_chain_decision = plan.decisions().front();
    }
  };

  Plan::iterateAvailablePlans(CoreField(field), seq, 2, dicisionMaker);
  return best_chain_grade < previous_chain_grade || strategy == FIRE
             ? DropDecision(best_fire_decision)
             : DropDecision(best_chain_decision);
}

void munetoshi::AI::enemyGrounded(const FrameData& frame) {
  CoreField field(frame.enemyPlayerFrameData().field);
  field.forceDrop();

  if (field.simulate().chains > 1) {
    strategy = FIRE;
  }
}

int munetoshi::AI::evaluate(const PlainField& field) {
  int grade = -1;
  int sum;
  auto adder = [&](std::tuple<int, PuyoColor> t) { sum += std::get<0>(t); };
  std::vector<TrackedPossibleRensaInfo> rensa_info_vect =
      RensaDetector::findPossibleRensasWithTracking(field, 1, RensaDetector::FLOAT);
  for (auto i = rensa_info_vect.begin(); i != rensa_info_vect.end(); ++i) {
    sum = 0;
    auto required_puyos = i->necessaryPuyoSet.list();
    std::for_each(required_puyos.rbegin(), required_puyos.rend(), adder);
    grade = std::max(grade, std::max(i->rensaResult.chains * 10 - sum * 3, 0));
  }
  return grade;
}
