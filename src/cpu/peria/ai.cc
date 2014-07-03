#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/plan.h"
#include "core/field/core_field.h"
#include "core/field/rensa_result.h"

namespace peria {

Ai::Ai(): ::AI("peria") {}

Ai::~Ai() {}

DropDecision Ai::think(int frame_id,
		       const PlainField& field,
		       const KumipuyoSeq& seq) {
  UNUSED_VARIABLE(frame_id);

  LOG(INFO) << CoreField(field).toDebugString() << seq.toString();
  
  Decision best;
  int score = -1;
  Plan::iterateAvailablePlans(
      CoreField(field),
      seq,
      2,
      [&best, &score](const RefPlan& plan) {
        int s = 0;
        if (plan.isRensaPlan()) {
          s = plan.rensaResult().score;
        }

        if (score < s) {
          score = s;
          best = plan.decisions().front();
        }
      });

  return DropDecision(best);
}

void Ai::gameWillBegin(const FrameData& /*frame_data*/) {
}

void Ai::enemyGrounded(const FrameData& /*frame_data*/) {
}

}  // namespace peria
