#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/base.h"
#include "core/plan/plan.h"
#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/frame_request.h"

class SampleAI : public AI {
public:
    SampleAI(int argc, char* argv[]) : AI(argc, argv, "sample") {}
    ~SampleAI() override {}

    // think() is the method that you need to implement.
    // frameId: the frame id when the puyo begins to move.
    // f: current field.
    // seq; current kumipuyo sequence. It should contain 2 kumipuyos at least.
    // me, enemy: the detail of each player state.
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

        // Do bruteforce search with depth 2. Finds the fireable largest rensa.
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
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

    SampleAI(argc, argv).runLoop();
    return 0;
}
