#include "cpu/peria/evaluate.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <sstream>

#include "core/algorithm/plan.h"
#include "core/field/core_field.h"
#include "core/kumipuyo_seq.h"

#include "cpu/peria/pattern.h"

namespace peria {

// Score current field situation.
int Evaluate::PatternMatch(const RefPlan& plan, std::string* name) {
  int sum = 0;
  int best = 0;

  const CoreField& field = plan.field();
  std::ostringstream oss;
  for (const Pattern& pattern : Pattern::GetAllPattern()) {
    int score = pattern.Match(field);
    sum += score;
    if (score > best) {
      best = score;
      oss << " " << pattern.name() << " " << score << "/" << pattern.score();
    }
  }
  *name = oss.str();

  // Debug list
  // LOG(INFO) << *name;
  // LOG(INFO) << "Sum: " << sum;
  // LOG(INFO) << "Num: " << field.countPuyos();
  // LOG(INFO) << "\n" << field.toDebugString();

  return sum;
}

void Evaluate::Future(const RefPlan& plan, int* best_score) {
  int score = plan.rensaResult().score;
  if (score > *best_score)
    *best_score = score;
}

void Evaluate::Usual(const RefPlan& plan,
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
  } else {
    Plan::iterateAvailablePlans(
        plan.field(), KumipuyoSeq(), 1,
        std::bind(Future, std::placeholders::_1, &score));
    score /= 2;
  }

  if (score > *best_score) {
    *best_score = score;
    *decision = plan.decisions().front();
  }
}

void Evaluate::Patterns(const RefPlan& plan,
                        int* best_score,
                        std::string* best_name,
                        int* frames,
                        Decision* decision) {
  // Do not check patterns if puyos vanish.
  if (plan.isRensaPlan())
    return;

  std::string name;
  int score = PatternMatch(plan, &name);
  if (score > *best_score ||
      (score == *best_score && plan.rensaResult().frames < *frames)) {
    *best_score = score;
    *best_name = name;
    *frames = plan.rensaResult().frames;
    *decision = plan.decision(0);
  }
}

void Evaluate::Counter(const RefPlan& plan,
                       int threshold,
                       int frame,
                       int* score,
                       Decision* decision) {
  if (!plan.isRensaPlan())
    return;
  if (plan.framesToInitiate() < frame)
    return;
  if (plan.score() < threshold)
    return;
  if (*score > 0 && *score <= plan.score())
    return;

  *score = plan.score();
  *decision = plan.decision(0);
}

}  // namespace peria

