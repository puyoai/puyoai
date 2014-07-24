#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>
#include <functional>  // C++11
#include <sstream>
#include <tuple>  // C++11
#include <vector>

#include "core/algorithm/plan.h"
#include "core/constant.h"
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

typedef std::tuple<int, bool, Decision> Candidate;

void EvaluateUsual(const RefPlan& plan, std::vector<Candidate>* candidates) {
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

  candidates->push_back(std::make_tuple(score, plan.isRensaPlan(), plan.decisions().front()));
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
  
  if (attack_ && attack_->end_frame_id < frame_id)
    attack_.reset();

  std::vector<Candidate> candidates;
  Plan::iterateAvailablePlans(CoreField(field), seq, seq.size() + 1,
                              std::bind(EvaluateUsual, _1, &candidates));
  std::sort(candidates.begin(), candidates.end());
  LOG(INFO) << "Candidates: " << candidates.size();

  // 3 個以上おじゃまが来てたらカウンターしてみる
  if (attack_ && attack_->score >= SCORE_FOR_OJAMA * 3) {
    // TODO: Adjust |kAcceptablePuyo|.
    // TODO: 受けられるかチェックする。
    const int kAcceptablePuyo = 3;
    int threshold = attack_->score - SCORE_FOR_OJAMA * kAcceptablePuyo;
    for (const auto& candidate : candidates) {
      int score = std::get<0>(candidate);
      if (score < threshold)
        continue;

      if (std::get<1>(candidate)) {
        const Decision& decision = std::get<2>(candidate);
        message << "SCORE:" << score
                << "_TO_COUNTER:" << attack_->score;
        return DropDecision(decision, message.str());
      }
    }
  }

  // TODO: Introduce Random class.
  const Candidate& cand = candidates[candidates.size() * 2 / 3];
  int score = std::get<0>(cand);
  message << "SCORE:" << score;
  if (std::get<1>(cand))
    message << "_FIRE";
  return DropDecision(std::get<2>(cand), message.str());
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
