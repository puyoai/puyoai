#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/base.h"
#include "core/plan/plan.h"
#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/frame_request.h"

class RyumasAI : public AI {
public:
    RyumasAI(int argc, char* argv[]) : AI(argc, argv, "ryumas") {}
    ~RyumasAI() override {}

    DropDecision think(int frameId, const CoreField& f, const KumipuyoSeq& seq,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const override
    {
        UNUSED_VARIABLE(frameId);
        UNUSED_VARIABLE(me);
        UNUSED_VARIABLE(enemy);
        UNUSED_VARIABLE(fast);

        LOG(INFO) << f.toDebugString() << seq.toString();

        Decision best;
        int score = -1;

        Plan::iterateAvailablePlans(f, seq, 2, [&best, &score](const RefPlan& plan) {
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
};

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    RyumasAI(argc, argv).runLoop();
    return 0;
}
