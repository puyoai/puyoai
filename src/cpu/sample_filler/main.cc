#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>

#include "core/algorithm/plan.h"
#include "core/client/ai/ai.h"
#include "core/field/core_field.h"

using namespace std;

class SampleFillerAI : public AI {
public:
    SampleFillerAI() : AI("sample-filler") {}
    virtual ~SampleFillerAI() {}

    virtual DropDecision think(int frameId, const PlainField& f, const KumipuyoSeq& seq) override
    {
        UNUSED_VARIABLE(frameId);
        UNUSED_VARIABLE(seq);

        const CoreField cf(f);

        pair<int, int> heights[] = {
            make_pair(cf.height(1), 1),
            make_pair(cf.height(6), 6),
            make_pair(cf.height(2), 2),
            make_pair(cf.height(5), 5),
            make_pair(cf.height(4), 4),
            make_pair(cf.height(3), 3),
        };

        sort(heights, heights + 6);
        for (int i = 0; i < 6; ++i) {
            CoreField cf2(cf);
            int x = heights[i].second;
            cf2.dropKumipuyo(Decision(x, 2), seq.get(0));
            RensaResult rr = cf2.simulate();
            if (rr.score == 0)
                return DropDecision(Decision(x, 2));
        }

        return DropDecision(Decision(3, 2));
    }
};

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    SampleFillerAI().runLoop();

    return 0;
}
