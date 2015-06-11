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
  UNUSED_VARIABLE(enemy_state);
  using namespace std::placeholders;

  //enemy_.state = enemy_state;

  Control control;
  Evaluator evaluator(my_state, enemy_, &control);
  Plan::iterateAvailablePlans(field, seq, 2, [&evaluator](const RefPlan& plan) { evaluator.EvalPlan(plan); });

  return DropDecision(control.decision, control.message);
}

}  // namespace peria
