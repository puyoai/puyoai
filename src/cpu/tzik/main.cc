
#include "main.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/plan.h"
#include "core/field/core_field.h"

namespace tzik {

AI::AI() : ::AI("tzik") {}

DropDecision AI::think(int frame_id,
                       const PlainField& field,
                       const Kumipuyo& next1,
                       const Kumipuyo& next2) {
  return think_sample(frame_id, field, next1, next2);
}

DropDecision AI::think_sample(int frame_id,
                              const PlainField& field,
                              const Kumipuyo& next1,
                              const Kumipuyo& next2) {
  UNUSED_VARIABLE(frame_id);
  KumipuyoSeq seq { next1, next2 };

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

    tzik::AI().runLoop();

    return 0;
}
