#include "cpu/peria/situation.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "core/kumipuyo_seq.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"

DEFINE_int32(simulate_size, 10, "The number of Kumipuyos to append in simulations.");
DEFINE_int32(simulate_width, 20, "Bandwidth in the beamseach");
DEFINE_double(ucb_coef, 10.0, "Contant coefficient in getting UCB");

namespace peria {

namespace {

struct State {
  CoreField field;
  bool isZenkeshi = false;

  // Total score to achieve this situation. |score| does not include expected score.
  int expectScore = 0;
  int score = 0;
  int frameId = 0;
};

std::vector<std::vector<State>> states;

}  // namespace

Situation::Situation(const Decision& firstDecision, const CoreField& field,
                     const int score, const int frameId)
  : decision_(firstDecision),
    field_(field),
    score_(score),
    frameId_(frameId)
{
  UNUSED_VARIABLE(frameId_);
}

Situation::~Situation() {}

void Situation::evaluate(const KumipuyoSeq& seq) {
  UNUSED_VARIABLE(seq);
  const int n = std::min(static_cast<int>(seq.size()), FLAGS_simulate_size);
  states.clear();
  states.resize(n + 1);
  {
    State st;
    st.field = field_;
    states[0].push_back(st);
  }

  for (int i = 0; i < n; ++i) {
    std::vector<State>& to = states[i + 1];
    for (State& from : states[i]) {
      auto iterateNext = [&from, &to](const RefPlan& plan) {
        int expectScore = 0;
        bool prohibits[FieldConstant::MAP_WIDTH] {};
        auto complementCallback = [&expectScore](CoreField&& field, const ColumnPuyoList&) {
          RensaResult result = field.simulate();
          expectScore = result.score;
        };
        RensaDetector::detectByDropStrategy(plan.field(), prohibits,
                                            PurposeForFindingRensa::FOR_FIRE, 2, 13,
                                            complementCallback);
        State st;
        st.field = plan.field();
        st.isZenkeshi = plan.hasZenkeshi();
        st.expectScore = expectScore;
        st.score = plan.score() + from.score;
        to.push_back(st);
      };
      Plan::iterateAvailablePlans(from.field, {seq.get(i)}, 1, iterateNext);
    }

    // No choise to be alive.
    if (to.size() == 0) {
      addScore(0);
      return;
    }

    sort(to.begin(), to.end(), [](const State& a, const State& b) -> bool {
        return a.expectScore + a.score > b.expectScore + b.score;
      });
    if (static_cast<int>(to.size()) > FLAGS_simulate_width) {
      to.resize(FLAGS_simulate_width);
    }
  }

  int bestScore = 0;
  for (auto& ss : states) {
    for (auto& s : ss) {
      bestScore = std::max(bestScore, s.score);
    }
  }

  addScore(bestScore);
}

void Situation::addScore(int value) {
  ++numTried_;
  sumScore_ += value;
  avgScore_ = static_cast<double>(sumScore_) / numTried_;
}

double Situation::ucb(int n) const {
  return score_ + avgScore_ + FLAGS_ucb_coef * std::sqrt(std::log(n) / numTried_);
}

}  // namespace peria
