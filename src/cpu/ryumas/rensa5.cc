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
    virtual ~RyumasAI() {}

    virtual DropDecision think(int frameId, const CoreField& f, const KumipuyoSeq& seq,
                               const PlayerState& me, const PlayerState& enemy, bool fast) const override
    {
        UNUSED_VARIABLE(frameId);
        UNUSED_VARIABLE(me);
        UNUSED_VARIABLE(enemy);
        return eval(f, seq, fast ? 2 : 3);
    }

private:
    DropDecision eval(const CoreField& f, const KumipuyoSeq& nexts, int depth) const
    {
        LOG(INFO) << f.toDebugString() << nexts.toString();

        Decision best = Decision(3, 0);
        int score = -100000;

        Plan::iterateAvailablePlans(f, nexts, depth, [&best, &score](const RefPlan& plan) {
            if (!plan.isRensaPlan())
                return;

            int s = 0;
            // When we can fire >=3 chain, we fire it.
            switch (plan.decisions().size()) {
            case 1:
                if (plan.rensaResult().chains >= 5)
                    s = plan.rensaResult().score + 1000000;
                break;
            case 2:
                s = plan.rensaResult().chains * 10000 + plan.rensaResult().score;
                break;
            case 3:
                s = plan.rensaResult().chains * 1000 + plan.rensaResult().score;
                break;
            }

            s -= plan.framesToIgnite();
            s -= plan.field().height(2) * 5;
            s -= plan.field().height(3) * 20; // Do not put 3rd column so much.
            s -= plan.field().height(4) * 10;
            s -= plan.field().height(5) * 5;
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
