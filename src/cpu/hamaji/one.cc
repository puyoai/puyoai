#include <limits.h>
#include <stdio.h>

#include <algorithm>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/base.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/frame_request.h"

using namespace std;

class One : public AI {
 public:
  One(int argc, char* argv[])
      : AI(argc, argv, "one") {}
  virtual ~One() {}

  virtual DropDecision think(int frameId,
                             const CoreField& f,
                             const KumipuyoSeq& seq,
                             const PlayerState& me,
                             const PlayerState& enemy,
                             bool fast) const override {
    UNUSED_VARIABLE(frameId);
    UNUSED_VARIABLE(me);
    UNUSED_VARIABLE(enemy);
    UNUSED_VARIABLE(fast);

    LOG(INFO) << f.toDebugString() << seq.toString();

    Decision best;
    int score = -1;

    Plan::iterateAvailablePlans(
        f, seq, 2, [&best, &score](const RefPlan& plan) {
          int s = -1;
          if (plan.decisions().size() == 1 && plan.isRensaPlan() &&
              plan.rensaResult().score > 2000) {
            s = plan.rensaResult().score;
            if (score < s) {
              score = s;
              best = plan.decisions().front();
            }
          }
        });
    if (best.isValid()) {
      char buf[256];
      sprintf(buf, "fire %d", score);
      return DropDecision(best, buf);
    }

    score = INT_MIN;
    Plan::iterateAvailablePlans(
        f, seq, 2, [&best, &score](const RefPlan& plan) {
          int s = -1;
          auto callback = [&](CoreField&& cf, const ColumnPuyoList&) -> RensaResult {
            RensaResult rensa_result = cf.simulate();
            s = max(s, rensa_result.score);
            return rensa_result;
          };
          RensaDetector::detectIteratively(
              plan.field(), RensaDetectorStrategy::defaultFloatStrategy(), 2, callback);

          if (plan.isRensaPlan()) {
            if (plan.decisions().size() == 1) {
              s -= plan.rensaResult().score;
            } else if (plan.rensaResult().score > 2000) {
              s += plan.rensaResult().score * 10;
            }
          }

          if (score < s) {
            score = s;
            best = plan.decisions().front();
          }
        });

    return DropDecision(best);
  }
};

int main(int argc, char* argv[])
{
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  One(argc, argv).runLoop();
  return 0;
}
