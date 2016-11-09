#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <climits>
#include <cstdint>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "base/base.h"
#include "base/time.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/pattern/decision_book.h"
#include "core/core_field.h"
#include "core/frame_request.h"
#include "core/player_state.h"

#include "search_without_risk.h"
#include "control.h"
#include "evaluator.h"
#include "player_hands.h"
#include "pattern.h"

namespace peria {

Ai::Ai(int argc, char* argv[]): ::AI(argc, argv, "peria") {}

Ai::~Ai() {}

DropDecision Ai::think(int frame_id,
                       const CoreField& field,
                       const KumipuyoSeq& seq,
                       const PlayerState& me,
                       const PlayerState& enemy,
                       bool fast) const {
  UNUSED_VARIABLE(fast);

  Control control;
  control.decision= Decision(3, 2);
  control.message = "No choice";
  control.score = -10000;

  // Check if it is in Joski template.
  DropDecision joseki = checkJoseki(field, seq);
  if (joseki.isValid()) {
    return joseki;
  }

  // Check if the enemy is firing the main rensa (Honsen)
  if (SearchWithoutRisk::shouldRun(enemy)) {
    SearchWithoutRisk search(me, seq, enemy.rensaFinishingFrameId() - frame_id);
    Decision decision = search.run();
    std::ostringstream oss;
    if (decision.isValid()) {
      oss << "Honki mode\n";
    } else {
      oss << "Honki muri\n";
      decision = Decision(3, 2);
    }
    oss << "Beam run " << search.countBeam() << " times";
    return DropDecision(decision, oss.str());
  }

  Evaluator evaluator(me, enemy, enemy_hands_, &control);

  // if enemy is firing a rensa, we use its after state.
  PlayerState me2(me);
  PlayerState enemy2(enemy);
  if (enemy2.isRensaOngoing()) {
    if (enemy2.hasZenkeshi)
      me2.pendingOjama += 6 * 5;
    enemy2.hasZenkeshi = enemy2.field.isZenkeshi();
    int use_score = enemy2.unusedScore + enemy2.currentRensaResult.score;
    me2.pendingOjama += use_score / 70;
    enemy2.unusedScore = use_score % 70;
  }
  enemy2.fixedOjama += enemy2.pendingOjama;
  enemy2.pendingOjama = 0;
  auto callback = std::bind(IterationCallback,
                            0, frame_id, me2, enemy2, seq.subsequence(1),
                            evaluator, std::placeholders::_1);

  std::int64_t start_ms = currentTimeInMillis();
  Plan::iterateAvailablePlans(field, seq, 1, callback);
  std::int64_t end_ms = currentTimeInMillis();

  std::ostringstream oss;
  oss << "\nThinkTime: " << (end_ms - start_ms) << "ms";
  control.message += oss.str();

  return DropDecision(control.decision, control.message);
}

void Ai::onGroundedForEnemy(const FrameRequest& frame_request) {
  const PlayerFrameRequest& enemy = frame_request.enemyPlayerFrameRequest();
  CoreField cf(CoreField::fromPlainFieldWithDrop(enemy.field));

  std::vector<PossibleRensa>& hands = enemy_hands_.firables;
  hands.clear();
  Plan::iterateAvailablePlans(
      cf, enemy.kumipuyoSeq, 2,
      [&frame_request, &hands](const RefPlan& plan) {
        if (plan.chains()) {
          PossibleRensa rensa;
          auto& enemy = frame_request.enemyPlayerFrameRequest();
          rensa.frames = plan.totalFrames();
          rensa.score = plan.score() + ((enemy.field.isZenkeshi() && enemy.score >= 40) ? ZENKESHI_BONUS : 0);
          rensa.num_required_puyos = 0;
          hands.push_back(rensa);
        }
      });

  std::vector<PossibleRensa>& possible = enemy_hands_.need_keys;
  possible.clear();
  RensaDetector::detectIteratively(
      cf, RensaDetectorStrategy::defaultFloatStrategy(), 2,
      [&possible](CoreField&& cf, const ColumnPuyoList& list) -> RensaResult {
        PossibleRensa rensa;
        RensaResult result = cf.simulate();
        rensa.frames = result.frames;
        rensa.score = result.score;
        rensa.num_required_puyos = list.size();
        possible.push_back(rensa);
        return result;
      });
}

DropDecision Ai::checkJoseki(const CoreField& field, const KumipuyoSeq& seq) const {
  DecisionBook* joseki = Pattern::getJoseki();
  DCHECK(joseki);
  Decision decision = joseki->nextDecision(field, seq);
  return DropDecision(decision, "By JOSEKI book");
}

namespace {

int simulateOjama(int ojama, CoreField& field) {
  int fall_ojama = std::min(ojama, 5 * 6);
  field.fallOjama(fall_ojama / 6);
  int x[] = {1,2,3,4,5,6};
  std::random_shuffle(x, x + 6);
  for (int i = 0; i < fall_ojama % 6; ++i) {
    field.dropPuyoOn(x[i], PuyoColor::OJAMA);
  }
  return ojama - fall_ojama;
}

}

// static
void Ai::IterationCallback(int step, int start_frame, PlayerState me, PlayerState enemy,
                           const KumipuyoSeq& next, Evaluator& evaluator, const RefPlan& plan) {
  int end_frame = start_frame + plan.totalFrames();
  me.field = plan.field();
  if (plan.isRensaPlan()) {
    if (me.hasZenkeshi)
      enemy.fixedOjama += 5 * 6;
    me.hasZenkeshi = me.field.isZenkeshi();
    me.currentRensaResult = plan.rensaResult();
    int use_score = me.unusedScore + me.currentRensaResult.score;
    enemy.fixedOjama += use_score / 70;
    me.unusedScore = use_score % 70;
  }

  // Update ojama status
  int sending_ojama = enemy.fixedOjama;
  int receiving_ojama = me.pendingOjama + me.fixedOjama;
  if (sending_ojama > receiving_ojama) {
    me.pendingOjama = 0;
    me.fixedOjama = 0;
    enemy.fixedOjama = sending_ojama - receiving_ojama;
  } else if (sending_ojama > me.fixedOjama) {
    me.fixedOjama = 0;
    me.pendingOjama = receiving_ojama - sending_ojama;
    enemy.fixedOjama = 0;
  } else { // sending_ojama < me.fixedOjama
    me.fixedOjama -= sending_ojama;;
    enemy.fixedOjama = 0;
  }

  // If enemy's rensa finishes, my pending ojamas are fixed.
  if (enemy.rensaFinishingFrameId() < end_frame) {
    me.fixedOjama += me.pendingOjama;
    me.pendingOjama = 0;
  }

  // Simulate ojama falls
  me.fixedOjama = simulateOjama(me.fixedOjama, me.field);
  enemy.fixedOjama = simulateOjama(enemy.fixedOjama, enemy.field);

  // If I die, it should not be evaluated.
  if (!me.field.isEmpty(3, 12))
    return;

  if (step == 0) {
    evaluator.setDecision(plan.decisions().front());
  }

  // Iterate more.
  if (step < 1 && next.size()) {
    auto callback = std::bind(IterationCallback,
                              step + 1, end_frame, me, enemy, next.subsequence(1), evaluator,
                              std::placeholders::_1);
    Plan::iterateAvailablePlans(me.field, next, 1, callback);
  } else {
    evaluator.EvalPlan(me, enemy, plan);
  }
}

}  // namespace peria
