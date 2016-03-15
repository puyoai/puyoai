#include "cpu/peria/situation.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "core/kumipuyo_seq.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"

DEFINE_int32(simulate_size, 20, "The number of Kumipuyos to append in simulations.");
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

  int expectScore = 0;
  bool prohibits[FieldConstant::MAP_WIDTH] {};
  auto complementCallback = [&expectScore](CoreField&& field, const ColumnPuyoList&) {
    RensaResult result = field.simulate();
    expectScore = result.score;
  };
  RensaDetector::detectByDropStrategy(field_, prohibits,
                                      PurposeForFindingRensa::FOR_FIRE, 2, 13,
                                      complementCallback);
  addScore(expectScore);
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
