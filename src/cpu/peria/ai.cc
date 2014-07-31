#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>
#include <functional>  // C++11
#include <map>
#include <sstream>
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
int PatternMatch(const RefPlan& plan, std::string* name) {
  int sum = 0;
  int best = 0;
  for (const Pattern& pattern : Pattern::GetAllPattern()) {
    int score = pattern.Match(plan.field());
    sum += score;
    if (score > best) {
      best = score;
      *name = pattern.name();
    }
  }
  return sum;
}

typedef std::map<Decision, std::vector<int> > CandidateMap;

void EvaluateUsual(const RefPlan& plan,
                   int* best_score,
                   Decision* decision) {
  int score = 0;
  if (plan.isRensaPlan()) {
    const RensaResult& result = plan.rensaResult();
    if (result.chains == 1 && result.quick && result.score > 70 * 6 * 2) {
      // Quick attack
      score = 99999;
    } else {
      score = result.score;
    }
  }

  if (score > *best_score) {
    *best_score = score;
    *decision = plan.decisions().front();
  }
}

void EvaluatePatterns(const RefPlan& plan,
                      int* best_score,
                      std::string* best_name,
                      int* frames,
                      Decision* decision) {
  std::string name;
  int score = PatternMatch(plan, &name);
  if (score > *best_score ||
      (score == *best_score && plan.rensaResult().frames < *frames)) {
    *best_score = score;
    *best_name = name;
    *frames = plan.rensaResult().frames;
    *decision = plan.decisions().front();
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
  
  if (attack_ && attack_->end_frame_id < frame_id)
    attack_.reset();

  int score = 0;
  std::string name;
  int frames = 1e+8;
  Decision decision;
  for (int n = 1; n <= seq.size(); ++n) {
    // Check templates first with visible puyos.
    Plan::iterateAvailablePlans(CoreField(field), seq, n,
                                std::bind(EvaluatePatterns, _1,
                                          &score, &name, &frames, &decision));
    if (score > 0 && !name.empty())
      return DropDecision(decision, "Template: " + name);
  }

  Plan::iterateAvailablePlans(CoreField(field), seq, seq.size() + 1,
                              std::bind(EvaluateUsual, _1, &score, &decision));

  // NOTE: 今は名前が違うだけ
  // 3 個以上おじゃまが来てたらカウンターしてみる
  if (attack_ && attack_->score >= SCORE_FOR_OJAMA * 3) {
    // TODO: Adjust |kAcceptablePuyo|.
    // TODO: 受けられるかチェックする。
    const int kAcceptablePuyo = 3;
    int threshold = attack_->score - SCORE_FOR_OJAMA * kAcceptablePuyo;
    if (threshold < score)
      return DropDecision(decision, "Counter");
  }

  return DropDecision(decision, "Normal");
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
