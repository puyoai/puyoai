#include "cpu/peria/pai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <vector>

#include "base/time.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/plan/plan.h"
#include "cpu/peria/situation.h"

DECLARE_int32(simulate_size);

namespace peria {

using int64 = std::int64_t;

Pai::Pai(int argc, char* argv[]): ::AI(argc, argv, "peria") {}

Pai::~Pai() {}

DropDecision Pai::think(int frameId,
                        const CoreField& field,
                        const KumipuyoSeq& seq,
                        const PlayerState& myState,
                        const PlayerState& enemyState,
                        bool fast) const {
  UNUSED_VARIABLE(myState);
  UNUSED_VARIABLE(enemyState);
  UNUSED_VARIABLE(fast);

  int64 startTime = currentTimeInMillis();

  // - Generate 22*22 states with iterating 2 known Kumipuyos
  //   - simulating enemy's firing chains not to die.
  std::vector<Situation> situations;
  auto iterateFirst = [&frameId, &seq, &situations](const RefPlan& firstPlan) {
    auto iterateSecond = [&frameId, &seq, &firstPlan, &situations](const RefPlan& plan) {
      Situation situation(firstPlan.firstDecision(), plan.field(),
                          firstPlan.score() + plan.score(), 
                          frameId + firstPlan.totalFrames() + plan.totalFrames());
      situations.push_back(situation);
    };
    Plan::iterateAvailablePlans(firstPlan.field(), seq.subsequence(1), 1, iterateSecond);
  };
  Plan::iterateAvailablePlans(field, seq, 1, iterateFirst);

  if (!situations.size()) {
    return DropDecision(Decision(3, 0), "Die");
  }

  KumipuyoSeq knownSeq = seq.subsequence(2);
  // Evaluate once
  for (auto& st : situations) {
    KumipuyoSeq s(knownSeq);
    int simulateSteps = FLAGS_simulate_size - s.size();
    simulateSteps = std::max(simulateSteps, 2);
    s.append(KumipuyoSeqGenerator::generateRandomSequence(simulateSteps));
    st.evaluate(s);
  }

  // - Run Monte Carlo search from each state
  //   - Use a beam search with a narrow width (~20?)
  //   - Choose start state whose UCB is the highest.
  //   - UCB[i] = average(past_score_from(i)) + C * sqrt(log(total_n) / n[i])
  int n = situations.size();
  sort(situations.begin(), situations.end(),
       [&n](const Situation& a, const Situation& b){
         return a.ucb(n) > b.ucb(n);
       });
  for (int i = 0; i < 2; ++i) {
    KumipuyoSeq s(knownSeq);
    int simulateSteps = FLAGS_simulate_size - s.size();
    simulateSteps = std::max(simulateSteps, 2);
    s.append(KumipuyoSeqGenerator::generateRandomSequence(simulateSteps));
    situations.front().evaluate(s);
    sort(situations.begin(), situations.end(),
         [&n](const Situation& a, const Situation& b){
           return a.ucb(n) > b.ucb(n);
         });
  }

  int64 endTime = currentTimeInMillis();
  std::ostringstream oss;
  oss << "ThinkTime: " << (endTime - startTime) << "ms";

  return DropDecision(situations.front().decision(), oss.str());
}

}  // namespace peria
