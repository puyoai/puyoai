#include "main.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/base.h"
#include "core/plan/plan.h"
#include "core/core_field.h"

namespace tzik {

AI::AI(int argc, char** argv) : ::AI(argc, argv, "tzik") {}

DropDecision AI::think(int frame_id,
                       const CoreField& field,
                       const KumipuyoSeq& seq,
                       const PlayerState& me,
                       const PlayerState& enemy,
                       bool fast) const {
  UNUSED_VARIABLE(me);
  UNUSED_VARIABLE(enemy);
  UNUSED_VARIABLE(fast);
  return think_sample(frame_id, field, seq);
}

DropDecision AI::think_sample(int frame_id,
                              const CoreField& field,
                              const KumipuyoSeq& seq) const {
  UNUSED_VARIABLE(frame_id);

  Decision best;
  int score = -1;

  Plan::iterateAvailablePlans(field, seq, 2,
                              [&](const RefPlan& plan) {
    int s = 0;
    if (plan.isRensaPlan()) {
      s += plan.rensaResult().chains * 10;
      s -= plan.decisions().size();
    }

    if (score < s) {
      score = s;
      best = plan.decisions().front();
    }
  });

  return DropDecision(best);
}

}  // namespace tzik

int main(int argc, char** argv)
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    tzik::AI(argc, argv).runLoop();

    return 0;
}
