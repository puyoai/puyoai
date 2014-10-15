
#include "main.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/plan.h"
#include "core/field/core_field.h"

namespace tzik {

AI::AI(int argc, char** argv) : ::AI(argc, argv, "tzik") {}

DropDecision AI::think(int frame_id,
                       const CoreField& field,
                       const KumipuyoSeq& seq,
                       const AdditionalThoughtInfo& info,
                       bool fast) {
  UNUSED_VARIABLE(info);
  UNUSED_VARIABLE(fast);
  return think_sample(frame_id, field, seq);
}

DropDecision AI::think_sample(int frame_id,
                              const PlainField& field,
                              const KumipuyoSeq& seq) {
  UNUSED_VARIABLE(frame_id);

  Decision best;
  int score = -1;

  Plan::iterateAvailablePlans(CoreField(field), seq, 2,
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
