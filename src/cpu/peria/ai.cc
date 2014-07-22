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

const int kDefaultThreshold = 70 * 6 * 8;

void Evaluate(const int threshold, Decision* best, int* score,
              const RefPlan& plan) {
  if (*score > threshold)
    return;

  int s = 0;
  if (plan.isRensaPlan()) {
    const RensaResult& result = plan.rensaResult();
    if (result.chains == 1 && result.quick && result.score > 70 * 6 * 2) {
      // Quick attack
      s = 99999;
    } else {
      s = result.score;
    }
  }
            
  if (*score < s) {
    *score = s;
    *best = plan.decisions().front();
  }
}

}  // namespace

struct Ai::Attack {
  int score;
  int end_frame_id;
};

// TODO: (want to implement)
// - Compute enemy's attack information on his settings.
// - Search decisions for all known |seq|
// --- Count the number of HAKKA-able KumiPuyos
// - Make patterns for JOSEKI.
// --- May be good to score all JOSEKI patterns and evaluate with $\sum score^2$

Ai::Ai(): ::AI("peria") {}

Ai::~Ai() {}

DropDecision Ai::think(int frame_id,
		       const PlainField& field,
		       const KumipuyoSeq& seq) {
  using namespace std::placeholders;

  std::ostringstream message;

  Decision best;
  int score = -1;

  if (attack_ && attack_->end_frame_id < frame_id)
    attack_.reset();

  int threashold = (attack_) ? (attack_->score + 1) : kDefaultThreshold;

  int depth = seq.size();
  Plan::iterateAvailablePlans(CoreField(field), seq, depth,
      std::bind(Evaluate, threashold, &best, &score, _1));

  message << "SCORE:" << score;
  if (attack_)
    message << "_COUNTER:" << attack_->score << "_IN_"
            << (attack_->end_frame_id - frame_id);

  return DropDecision(best, message.str());
}

void Ai::gameWillBegin(const FrameData& /*frame_data*/) {
  attack_.reset();
}

void Ai::enemyGrounded(const FrameData& frame_data) {
  const PlainField& enemy = frame_data.enemyPlayerFrameData().field;
  CoreField field(enemy);
  field.forceDrop();
  RensaResult result = field.simulate();

  if (result.chains == 0) {
    // TODO: Check required puyos to start RENSA.
    attack_.reset();
    return;
  }

  attack_.reset(new Attack);
  attack_->score = result.score;
  attack_->end_frame_id = frame_data.id + result.frames;
}

}  // namespace peria
