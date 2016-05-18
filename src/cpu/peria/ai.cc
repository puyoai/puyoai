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

namespace {
struct Attack {
  int ojama;
  int end_frame;
};
};

Ai::Ai(int argc, char* argv[]): ::AI(argc, argv, "peria") {}

Ai::~Ai() {}

DropDecision Ai::think(int frame_id,
                       const CoreField& field,
                       const KumipuyoSeq& seq,
                       const PlayerState& my_state,
                       const PlayerState& enemy_state,
                       bool fast) const {
  UNUSED_VARIABLE(frame_id);
  UNUSED_VARIABLE(fast);
  using namespace std::placeholders;

  std::int64_t start_ms = currentTimeInMillis();
  
  Control control;
  Evaluator evaluator(my_state, enemy_state, enemy_hands_, &control);
  Attack attack = {
    enemy_state.currentRensaResult.score / 70,
    enemy_state.rensaFinishingFrameId()
  };
  KumipuyoSeq next = seq.subsequence(1);
  auto simulateAndEval = [frame_id, attack, &next, &evaluator](const RefPlan& plan) {
    CoreField field = plan.field();
    int end_frame = frame_id + plan.totalFrames();
    if (frame_id <= attack.end_frame && attack.end_frame < end_frame) {
      int fall = attack.ojama - plan.score() / 70;
      if (fall < 0)
        fall = 0;
      if (fall > 30)
        fall = 30;
      field.fallOjama(fall / 6);
      int xs[] = {1,2,3,4,5,6};
      std::random_shuffle(xs, xs + 6);
      for (int i = 0; i < fall % 6; ++i) {
        field.dropPuyoOn(xs[i], PuyoColor::OJAMA);
      }
    }
    
    evaluator.EvalPlan(field, plan);
    if (!field.isEmpty(3, 12))
      return;
    auto callback = [&evaluator](const RefPlan& plan) { evaluator.EvalPlan(plan.field(), plan); };
    Plan::iterateAvailablePlans(field, next, 1, callback);
  };
  Plan::iterateAvailablePlans(field, seq, 1, simulateAndEval);
                              

  std::int64_t end_ms = currentTimeInMillis();

  std::ostringstream oss;
  oss << ",ThinkTime:_" << (end_ms - start_ms) << "ms";
  if (enemy_state.isRensaOngoing()) {
    const RensaResult& result = enemy_state.currentRensaResult;
    oss << ",Enemy:_Going(" << result.score << "_in_" << result.frames << ")";
  }
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

}  // namespace peria
