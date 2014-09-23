#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/plan.h"
#include "core/client/ai/ai.h"
#include "core/field/core_field.h"

class AIRoutine : public AI {
public:
    AIRoutine(int argc, char* argv[]) : AI(argc, argv, "sample") {}
    virtual ~AIRoutine() {}

    virtual DropDecision think(int frameId, const PlainField& f, const KumipuyoSeq& seq, const AdditionalThoughtInfo& info) override
    {
        UNUSED_VARIABLE(frameId);
        UNUSED_VARIABLE(info);

        LOG(INFO) << CoreField(f).toDebugString() << seq.toString();

        Decision best;
        int score = -1;

        Plan::iterateAvailablePlans(CoreField(f), seq, 2, [&best, &score](const RefPlan& plan) {
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

    AIRoutine(argc, argv).runLoop();
    return 0;
}
