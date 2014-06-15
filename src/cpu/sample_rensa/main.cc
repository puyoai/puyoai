#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/plan.h"
#include "core/client/ai/ai.h"
#include "core/field/core_field.h"

class SampleRensaAI : public AI {
public:
    SampleRensaAI() : AI("sample_rensa") {}
    virtual ~SampleRensaAI() {}

    virtual DropDecision thinkFast(int frameId, const PlainField& f, const Kumipuyo& next1, const Kumipuyo& next2) OVERRIDE
    {
        UNUSED_VARIABLE(frameId);
        KumipuyoSeq seq { next1, next2 };

        return eval(f, seq, 2);
    }

    virtual DropDecision think(int frameId, const PlainField& f, const Kumipuyo& next1, const Kumipuyo& next2) OVERRIDE
    {
        UNUSED_VARIABLE(frameId);
        KumipuyoSeq seq { next1, next2 };

        return eval(f, seq, 4);
    }

private:
    DropDecision eval(const PlainField& f, const KumipuyoSeq& nexts, int depth)
    {
        LOG(INFO) << CoreField(f).debugOutput() << nexts.toString();

        Decision best = Decision(3, 0);
        int score = -100000;

        Plan::iterateAvailablePlans(CoreField(f), nexts, depth, [&best, &score](const RefPlan& plan) {
            if (!plan.isRensaPlan())
                return;

            int s = 0;
            // When we can fire >=3 chain, we fire it.
            switch (plan.decisions().size()) {
            case 1:
                if (plan.rensaResult().chains >= 3)
                    s = plan.rensaResult().score + 1000000;
                break;
            case 2:
                s = plan.rensaResult().chains * 10000 + plan.rensaResult().score;
                break;
            case 3:
                s = plan.rensaResult().chains * 1000 + plan.rensaResult().score;
                break;
            }

            s -= plan.initiatingFrames();
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

    SampleRensaAI().runLoop();

    return 0;
}
