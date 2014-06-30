#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/plan.h"
#include "core/client/ai/ai.h"
#include "core/field/core_field.h"

class SampleSuicideAI : public AI {
public:
    SampleSuicideAI() : AI("sample-suicide") {}
    virtual ~SampleSuicideAI() {}

    virtual DropDecision think(int frameId, const PlainField& f, const KumipuyoSeq& seq) override
    {
        UNUSED_VARIABLE(frameId);
        UNUSED_VARIABLE(seq);

        CoreField cf(f);
        Decision d = cf.height(3) > 6 ? Decision(3, 0) : Decision(3, 2);
        return DropDecision(d);
    }
};

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    SampleSuicideAI().runLoop();

    return 0;
}
