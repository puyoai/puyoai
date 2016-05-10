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

  double bestScore = -1e100;
  Decision bestDecision(3, 2);

  auto evalFirst = [&bestScore, &bestDecision, &seq, &myState, &enemyState, &frameId](const RefPlan& plan) {
    const Decision& firstDecision = plan.firstDecision();
    CoreField field = plan.field();
    double score = plan.score();
    int64 frame = frameId + plan.totalFrames();
    if (enemyState.isRensaOngoing() && frame > enemyState.rensaFinishingFrameId()) {
      int64 ojama = myState.totalOjama(enemyState);
      if (ojama > 30) ojama = 30;
      field.fallOjama((ojama + 3) / 6);
    }
    if (!field.isEmpty(3, 12))
      return;

    auto evalSecond = [&bestScore, &bestDecision, &firstDecision](const RefPlan& plan) {
      if (plan.isRensaPlan() && plan.score() > bestScore) {
        bestDecision = firstDecision;
        bestScore = plan.score();
      }
    };
    Plan::iterateAvailablePlans(field, {seq.get(1)}, 1, evalSecond);

    if (score > bestScore) {
      bestDecision = firstDecision;
      bestScore = score;
    }
  };
  Plan::iterateAvailablePlans(field, {seq.get(0)}, 1, evalFirst);

  int64 endTime = currentTimeInMillis();

  oss << "Time : " << (endTime - startTime) << " / " << (dueTime - startTime) << "\n"
      << "Score : " << bestScore;
  return DropDecision(bestDecision, oss.str());
}

}  // namespace peria
