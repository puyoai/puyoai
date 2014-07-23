#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sstream>
#include <functional>

#include "core/algorithm/plan.h"
#include "core/field/core_field.h"
#include "core/field/rensa_result.h"
#include "core/puyo_color.h"
#include "pattern.h"

namespace peria {

namespace {

// Score current field situation.
int PatternMatch(const RefPlan& plan) {
  int sum = 0;
  for (const Pattern& pattern : Pattern::GetAllPattern()) {
    sum += pattern.Match(plan.field());
  }
  return sum;
}

void EvaluateUsual(Decision* best, int* best_score, const RefPlan& plan) {
  int score = 0;
  if (plan.isRensaPlan()) {
    const RensaResult& result = plan.rensaResult();
    if (result.chains == 1 && result.quick && result.score > 70 * 6 * 2) {
      // Quick attack
      score = 99999;
    } else {
      score = result.score;
    }
  } else {
    score = PatternMatch(plan);
  }
            
  if (*best_score < score) {
    *best_score = score;
    *best = plan.decisions().front();
  }
}

}  // namespace

struct Ai::Attack {
  int score;
  int end_frame_id;
};

// TODO: (want to implement)
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

  Plan::iterateAvailablePlans(CoreField(field), seq, seq.size(),
      std::bind(EvaluateUsual, &best, &score, _1));

  // Counter if OJAMA puyos return certainly.
  // TODO: Judge if AI should accept few OJAMA puyos. (受けるべきか?)
  if (attack_ && attack_->score < score) {
    message << "SCORE:" << score
            << "_TO_COUNTER:" << attack_->score;
    return DropDecision(best, message.str());
  }

  // Search best control.
  // TODO: Compute the possibility to get the best score.
  Plan::iterateAvailablePlans(CoreField(field), seq, seq.size() + 1,
      std::bind(EvaluateUsual, &best, &score, _1));

  message << "SCORE:" << score;
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
