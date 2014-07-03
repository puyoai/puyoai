#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sstream>

#include "core/algorithm/plan.h"
#include "core/field/core_field.h"
#include "core/field/rensa_result.h"
#include "core/puyo_color.h"

namespace peria {

Ai::Ai(): ::AI("peria") {}

Ai::~Ai() {}

DropDecision Ai::think(int frame_id,
		       const PlainField& field,
		       const KumipuyoSeq& seq) {
  UNUSED_VARIABLE(frame_id);

  LOG(INFO) << CoreField(field).toDebugString() << seq.toString();
  std::ostringstream message;

  Decision best;
  int score = -1;
  Plan::iterateAvailablePlans(CoreField(field), seq, 2,
      [&best, &score](const RefPlan& plan) {
        if (score > 70 * 5 * 12)
          return;

        int s = 0;
        if (plan.isRensaPlan()) {
          s = plan.rensaResult().score;
        }
            
        if (score < s) {
          score = s;
          best = plan.decisions().front();
        }
      });


  int depth = seq.size();
  KumipuyoSeq seq2(seq);
  seq2.resize(depth + 1);
  for (uint8_t i = RED; i <= GREEN; ++i) {
    seq2.setAxis(depth, static_cast<PuyoColor>(i));
    for (uint8_t j = RED; j <= GREEN; ++j) {
      seq2.setChild(depth, static_cast<PuyoColor>(j));
  Plan::iterateAvailablePlans(CoreField(field), seq2, depth + 1,
      [&best, &score](const RefPlan& plan) {
        if (score > 70 * 5 * 12)
          return;

        int s = 0;
        if (plan.isRensaPlan()) {
          s = plan.rensaResult().score;
        }
            
        if (score < s) {
          score = s;
          best = plan.decisions().front();
        }
      });
    }
  }
  message << "X-R:" << best.x << "-" << best.r
          << "_SCORE:" << score << "   ";

  return DropDecision(best, message.str());
}

void Ai::gameWillBegin(const FrameData& /*frame_data*/) {
}

void Ai::enemyGrounded(const FrameData& /*frame_data*/) {
}

}  // namespace peria
