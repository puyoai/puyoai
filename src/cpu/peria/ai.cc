#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <climits>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "base/base.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_detector.h"
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
  UNUSED_VARIABLE(frame_id);
  UNUSED_VARIABLE(fast);
  using namespace std::placeholders;

  Control control;
  Evaluator evaluator(my_state, enemy_state, enemy_hands_, &control);
  Plan::iterateAvailablePlans(field, seq, 2,
                              [&evaluator](const RefPlan& plan) { evaluator.EvalPlan(plan); });

  return DropDecision(control.decision, control.message);
}

void Ai::onGroundedForEnemy(const FrameRequest& frame_request) {
  const PlayerFrameRequest& enemy = frame_request.enemyPlayerFrameRequest();
  CoreField cf(enemy.field);

  std::vector<PossibleRensa>& hands = enemy_hands_.firables;
  hands.clear();
  Plan::iterateAvailablePlans(cf, enemy.kumipuyoSeq, 2,
                              [&hands](const RefPlan& plan) {
                                if (plan.chains()) {
                                  PossibleRensa rensa;
                                  rensa.frames = plan.totalFrames();
                                  rensa.score = plan.score();
                                  rensa.num_required_puyos = 0;
                                  hands.push_back(rensa);
                                }
                              });

  std::vector<PossibleRensa>& possible = enemy_hands_.need_keys;
  possible.clear();
  RensaDetector::detectIteratively(cf, RensaDetectorStrategy::defaultFloatStrategy(), 2,
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
