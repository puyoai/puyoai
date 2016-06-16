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
                       const PlayerState& my_state,
                       const PlayerState& enemy_state,
                       bool fast) const {
  UNUSED_VARIABLE(fast);

  Control control;
  Evaluator evaluator(my_state, enemy_state, enemy_hands_, &control);
  Attack attack = {
    enemy_state.currentRensaResult.score / 70,
    enemy_state.rensaFinishingFrameId()
  };

  auto callback = std::bind(IterationCallback, 0, frame_id, attack, seq.subsequence(1),
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

// static
void Ai::IterationCallback(int step,
                           int start_frame, Attack attack, const KumipuyoSeq& next,
                           Evaluator& evaluator, const RefPlan& plan) {
  CoreField field = plan.field();
  int playable_frame = start_frame + plan.totalFrames();

  // TODO: Take Zenkeshi into consideration.
  int sending_ojama = plan.score() / 70;
  if (start_frame <= attack.end_frame && attack.end_frame < playable_frame) {
    int falling_ojama = attack.ojama - sending_ojama;
    // If my attack is stronger than enemy's.
    if (falling_ojama < 0) {
      falling_ojama = 0;
      sending_ojama -= attack.ojama;
      attack.ojama = 0;
    }
    // If enemy's attack is very string.
    if (falling_ojama > 30) {
      falling_ojama = 30;
    }
    // If enemy's attack is stronger than mine.
    if (falling_ojama > 0) {
      sending_ojama = 0;
      attack.ojama -= falling_ojama;

      // Simulate ojama puyos.
      field.fallOjama(falling_ojama / 6);
      int xs[] = {1, 2, 3, 4, 5, 6};
      std::random_shuffle(xs, xs + 6);
      for (int i = 0; i < falling_ojama % 6; ++i) {
        field.dropPuyoOn(xs[i], PuyoColor::OJAMA);
      }
    }
  }
  
  if (!field.isEmpty(3, 12))
    return;

  if (step == 0) {
    evaluator.setDecision(plan.decisions().front());
  }
  evaluator.EvalPlan(field, sending_ojama, plan);

  // Iterate more.
  if (step < 1 && next.size()) {
    // TODO: Count control frames into playable_frame
    auto callback = std::bind(IterationCallback,
                              step + 1, playable_frame, attack, next.subsequence(1), evaluator,
                              std::placeholders::_1);
    Plan::iterateAvailablePlans(field, next, 1, callback);
  }
}

}  // namespace peria
