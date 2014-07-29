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
int PatternMatch(const RefPlan& plan) {
  int sum = 0;
  for (const Pattern& pattern : Pattern::GetAllPattern()) {
    sum += pattern.Match(plan.field());
  }
  return sum;
}

typedef std::map<Decision, std::vector<int> > CandidateMap;

void EvaluateUsual(const RefPlan& plan, CandidateMap* candidates) {
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

  const Decision& decision = plan.decisions().front();
  if (candidates->find(decision) == candidates->end())
    candidates->insert(std::make_pair(decision, std::vector<int>()));
  (*candidates)[decision].push_back(score);
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

  CandidateMap candidates;
  Plan::iterateAvailablePlans(CoreField(field), seq, seq.size() + 1,
                              std::bind(EvaluateUsual, _1, &candidates));

  LOG(INFO) << "Candidates: " << candidates.size();

  // 3 個以上おじゃまが来てたらカウンターしてみる
  if (attack_ && attack_->score >= SCORE_FOR_OJAMA * 3) {
    // TODO: Adjust |kAcceptablePuyo|.
    // TODO: 受けられるかチェックする。
    const int kAcceptablePuyo = 3;
    int threshold = attack_->score - SCORE_FOR_OJAMA * kAcceptablePuyo;
    for (const auto& candidate : candidates) {
      const Decision decision = candidate.first;
      const auto& scores = candidate.second;
      int score = *std::max_element(scores.begin(), scores.end());

      if (score < threshold)
        continue;

      return DropDecision(decision, message.str());
    }
  }

  auto best_candidate = candidates.begin();
  int best_avg = -1;
  bool rand_hand = false;
  for (auto itr = candidates.begin(); itr != candidates.end(); ++itr) {
    if (itr->second.empty())
      continue;
    int sum = 0;
    for (int score : itr->second) {
      sum += std::max(0, score - 20);
    }

    int avg = sum / itr->second.size();
    if (avg >= best_avg) {
      best_avg = avg;
      best_candidate = itr;
      rand_hand = false;
    } else if (best_avg == avg && rand() % 2 == 0) {
      best_avg = avg;
      best_candidate = itr;
      rand_hand = true;
    }
  }

  return DropDecision(best_candidate->first, rand_hand ? "Randomize" : "Normal");
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
