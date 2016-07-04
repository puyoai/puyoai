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
#include "core/core_field.h"
#include "core/frame_request.h"
#include "core/player_state.h"
#include "cpu/peria/control.h"
#include "cpu/peria/evaluator.h"
#include "cpu/peria/player_hands.h"

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
  control.score = 0;

  Evaluator evaluator(me, enemy, enemy_hands_, &control);

  // if enemy is firing a rensa, we use its after state.
  PlayerState enemy2(enemy);
  if (enemy2.isRensaOngoing()) {
    if (enemy2.hasZenkeshi)
      enemy2.unusedScore += ZENKESHI_BONUS;
    enemy2.hasZenkeshi = enemy2.field.isZenkeshi();
    enemy2.unusedScore += enemy2.currentRensaResult.score;
    enemy2.currentRensaResult.score = 0;
  }
  auto callback = std::bind(IterationCallback,
                            0, frame_id, me, enemy2, seq.subsequence(1),
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

namespace {

int simulateOjama(int score, CoreField& field) {
  int ojama = std::min(score / 70, 5 * 6);
  field.fallOjama(ojama / 6);
  int x[] = {1,2,3,4,5,6};
  std::random_shuffle(x, x + 6);
  for (int i = 0; i < ojama % 6; ++i) {
    field.dropPuyoOn(x[i], PuyoColor::OJAMA);
  }
  return score - ojama * 70;
}

}

// static
void Ai::IterationCallback(int step, int start_frame, PlayerState me, PlayerState enemy,
                           const KumipuyoSeq& next, Evaluator& evaluator, const RefPlan& plan) {
  int end_frame = start_frame + plan.totalFrames();
  me.field = plan.field();
  if (plan.isRensaPlan()) {
    if (me.hasZenkeshi)
      me.unusedScore += ZENKESHI_BONUS;
    me.hasZenkeshi = me.field.isZenkeshi();
    me.currentRensaResult = plan.rensaResult();
    me.unusedScore += me.currentRensaResult.score;
    me.currentRensaResult.score = 0;
  }

  if (me.unusedScore > enemy.unusedScore) {
    me.unusedScore -= enemy.unusedScore;
    enemy.unusedScore = 0;
    me.unusedScore -= simulateOjama(me.unusedScore, enemy.field);
  } else {
    enemy.unusedScore = me.unusedScore;
    me.unusedScore = 0;
    if (enemy.rensaFinishingFrameId() < end_frame) {
      enemy.unusedScore -= simulateOjama(enemy.unusedScore, me.field);
    }
  }

  // If I die, it should not be evaluated.
  if (!me.field.isEmpty(3, 12))
    return;

  if (step == 0) {
    evaluator.setDecision(plan.decisions().front());
  }
  evaluator.EvalPlan(me, enemy, plan);

  // Iterate more.
  if (step < 1 && next.size()) {
    auto callback = std::bind(IterationCallback,
                              step + 1, end_frame, me, enemy, next.subsequence(1), evaluator,
                              std::placeholders::_1);
    Plan::iterateAvailablePlans(me.field, next, 1, callback);
  }
}

}  // namespace peria
