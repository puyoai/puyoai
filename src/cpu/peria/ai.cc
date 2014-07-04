#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sstream>
#include <functional>

#include "core/algorithm/plan.h"
#include "core/field/core_field.h"
#include "core/field/rensa_result.h"
#include "core/puyo_color.h"

namespace peria {

namespace {

void Evaluate(const int threshold, Decision* best, int* score,
              const RefPlan& plan) {
  if (*score > threshold)
    return;

  int s = 0;
  if (plan.isRensaPlan()) {
    s = plan.rensaResult().score;
  }
            
  if (*score < s) {
    *score = s;
    *best = plan.decisions().front();
  }
}

}  // namespace

Ai::Ai(): ::AI("peria") {}

Ai::~Ai() {}

DropDecision Ai::think(int frame_id,
		       const PlainField& field,
		       const KumipuyoSeq& seq) {
  using namespace std::placeholders;
  UNUSED_VARIABLE(frame_id);

  LOG(INFO) << CoreField(field).toDebugString() << seq.toString();
  std::ostringstream message;

  Decision best;
  int score = -1;

  int depth = seq.size();
  KumipuyoSeq seq2(seq);
  seq2.resize(depth + 1);
  for (uint8_t i = RED; i <= GREEN; ++i) {
    seq2.setAxis(depth, static_cast<PuyoColor>(i));
    for (uint8_t j = RED; j <= GREEN; ++j) {
      seq2.setChild(depth, static_cast<PuyoColor>(j));
      Plan::iterateAvailablePlans(CoreField(field), seq2, depth + 1,
          std::bind(Evaluate, 70 * 6 * 10, &best, &score, _1));
    }
  }

  Plan::iterateAvailablePlans(CoreField(field), seq, 2,
      std::bind(Evaluate, 70 * 6 * 5, &best, &score, _1));

  message << "X-R:" << best.x << "-" << best.r
          << "_SCORE:" << score << "   ";

  return DropDecision(best, message.str());
}

void Ai::gameWillBegin(const FrameData& /*frame_data*/) {
}

void Ai::enemyGrounded(const FrameData& /*frame_data*/) {
}

}  // namespace peria
