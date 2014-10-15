#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>

#include "core/algorithm/plan.h"
#include "core/client/ai/ai.h"
#include "core/field/core_field.h"
#include "core/kumipuyo_seq.h"
#include "core/puyo_controller.h"

using namespace std;

DEFINE_string(algorithm, "", "Choose algorithm: flat, peak, or nohoho");

class SampleFillerAI : public AI {
public:
    SampleFillerAI(int argc, char* argv[]) : AI(argc, argv, "sample-filler") {}
    virtual ~SampleFillerAI() {}

    virtual DropDecision think(int frameId, const CoreField& f, const KumipuyoSeq& seq,
                               const AdditionalThoughtInfo& info, bool fast) override
    {
        UNUSED_VARIABLE(info);
        UNUSED_VARIABLE(fast);

        if (FLAGS_algorithm == "peak") {
            return thinkPeak(frameId, f, seq);
        }
        if (FLAGS_algorithm == "nohoho") {
            return thinkNohoho(frameId, f, seq);
        }
        return thinkFlat(frameId, f, seq);
    }

private:
    DropDecision thinkFlat(int frameId, const CoreField& f, const KumipuyoSeq& seq)
    {
        UNUSED_VARIABLE(frameId);

        pair<int, int> heights[] = {
            make_pair(f.height(1), 1),
            make_pair(f.height(6), 6),
            make_pair(f.height(2), 2),
            make_pair(f.height(5), 5),
            make_pair(f.height(4), 4),
            make_pair(f.height(3), 3),
        };

        sort(heights, heights + 6);
        for (int i = 0; i < 6; ++i) {
            CoreField cf(f);
            int x = heights[i].second;
            cf.dropKumipuyo(Decision(x, 2), seq.get(0));
            RensaResult rr = cf.simulate();
            if (rr.score == 0)
                return DropDecision(Decision(x, 2));
        }

        return DropDecision(Decision(3, 2));
    }

    DropDecision thinkPeak(int frameId, const PlainField& f, const KumipuyoSeq& seq)
    {
        UNUSED_VARIABLE(frameId);

        static const int order[] = { 3, 2, 4, 5, 1, 6 };

        for (int i = 0; i < 6; ++i) {
            CoreField cf(f);
            int x = order[i];
            if (cf.height(x) >= 9)
                continue;
            if (!PuyoController::isReachable(f, Decision(x, 2)))
                continue;

            cf.dropKumipuyo(Decision(x, 2), seq.get(0));
            RensaResult rr = cf.simulate();
            if (rr.score == 0)
                return DropDecision(Decision(x, 2));
        }

        for (int i = 0; i < 6; ++i) {
            CoreField cf(f);
            int x = order[i];
            if (x == 3 && cf.height(x) <= 9)
                return DropDecision(Decision(3, 0));
            if (x != 3 && cf.height(x) <= 11)
                return DropDecision(Decision(x, 0));
        }

        return DropDecision(Decision(3, 2));
    }

    DropDecision thinkNohoho(int frameId, const CoreField& f, const KumipuyoSeq& seq)
    {
        UNUSED_VARIABLE(frameId);
        UNUSED_VARIABLE(seq);

        static const int order[] = { 6, 5, 4, 1, 2, 3 };

        for (int i = 0; i < 6; ++i) {
            const int x = order[i];
            if (PuyoController::isReachable(f, Decision(x, 2))) {
                return DropDecision(Decision(x, 2));
            }
            if (PuyoController::isReachable(f, Decision(x, 0))) {
                return DropDecision(Decision(x, 0));
            }
        }

        // Nothing reachable? Die.
        return DropDecision(Decision(3, 2));
    }
};

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    SampleFillerAI(argc, argv).runLoop();
    return 0;
}
