#include "cpu/peria/basic_ai.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/time.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/score.h"

namespace peria {

using int64 = std::int64_t;

BasicAi::BasicAi(int argc, char* argv[]) : ::AI(argc, argv, "peria") {}

BasicAi::~BasicAi() {}

DropDecision BasicAi::think(
    int frameId, const CoreField& field, const KumipuyoSeq& seq,
    const PlayerState& myState, const PlayerState& enemyState, bool fast) const {
  std::ostringstream oss;

  int64 startTime = currentTimeInMillis();
  int64 dueTime = startTime + (fast ? 30 : 300);

  double bestValue = -1e100;
  Decision bestDecision(3, 2);

  auto usualSearch = std::bind(UsualSearch,
                               seq.subsequence(1, 1), myState, enemyState, frameId, std::placeholders::_1,
                               &bestValue, &bestDecision);
  Plan::iterateAvailablePlans(field, seq, 1, usualSearch);

  int64 endTime = currentTimeInMillis();

  oss << "Time : " << (endTime - startTime) << " / " << (dueTime - startTime) << "\n"
      << "Score : " << bestValue;
  return DropDecision(bestDecision, oss.str());
}

// static
void BasicAi::UsualSearch(const KumipuyoSeq& seq, const PlayerState& myState, const PlayerState& enemyState, int frameId,
                          const RefPlan& plan, double* bestValue, Decision* bestDecision) {
  const Decision& firstDecision = plan.firstDecision();
  CoreField field = plan.field();
  double value = plan.chains() * 10000 + plan.score();

  int64 frame = frameId + plan.totalFrames();
  if (enemyState.isRensaOngoing() && frame > enemyState.rensaFinishingFrameId()) {
    int64 ojama = myState.totalOjama(enemyState);
    if (ojama > 30) ojama = 30;
    field.fallOjama((ojama + 3) / 6);
  }
  if (!field.isEmpty(3, 12))
    return;

  int expectScore = 0;
  int expectChain = 0;
  bool prohibits[FieldConstant::MAP_WIDTH] {};
  auto complementCallback = [&expectScore, &expectChain](CoreField&& field, const ColumnPuyoList&) {
      RensaResult result = field.simulate();
      expectScore = std::max(expectScore, result.score);
      expectChain = std::max(expectChain, result.chains);
  };
  RensaDetector::detectByDropStrategy(field, prohibits, PurposeForFindingRensa::FOR_FIRE, 2, 13, complementCallback);
  value += expectScore * 0.7 + expectChain * 2000;
  
  value -= (field.height(2) - field.height(1)) * 100;
  value -= (field.height(3) - field.height(2)) * 20;
  value -= (field.height(4) - field.height(5)) * 20;
  value -= (field.height(5) - field.height(6)) * 100;

  if (value > *bestValue) {
    *bestDecision = firstDecision;
    *bestValue = value;
  }

  auto evalSecond = [&bestValue, &bestDecision, &firstDecision](const RefPlan& plan) {
    CoreField field = plan.field();
    double value = plan.chains() * 10000 + plan.score();

    value -= (field.height(2) - field.height(1)) * 100;
    value -= (field.height(3) - field.height(2)) * 20;
    value -= (field.height(4) - field.height(5)) * 20;
    value -= (field.height(5) - field.height(6)) * 100;

    if (plan.isRensaPlan() && value > *bestValue) {
      *bestDecision = firstDecision;
      *bestValue = value;
    }
  };
  Plan::iterateAvailablePlans(field, seq, 1, evalSecond);
}

}  // namespace peria
