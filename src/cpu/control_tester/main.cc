#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>

#include "base/base.h"
#include "core/plan/plan.h"
#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq.h"
#include "core/puyo_controller.h"

using namespace std;

DEFINE_string(algorithm, "", "Choose algorithm: flat, peak, or nohoho");

class ControlTesterAI : public AI {
public:
    ControlTesterAI(int argc, char* argv[]) : AI(argc, argv, "control-tester") {}
    virtual ~ControlTesterAI() {}

    virtual DropDecision think(int frameId, const CoreField& f, const KumipuyoSeq& seq,
                               const PlayerState& me, const PlayerState& enemy, bool fast) const override
    {
        UNUSED_VARIABLE(me);
        UNUSED_VARIABLE(enemy);
        UNUSED_VARIABLE(fast);

        if (FLAGS_algorithm == "peak")
            return thinkPeak(frameId, f, seq);
        if (FLAGS_algorithm == "nohoho")
            return thinkNohoho(frameId, f, seq);
        if (FLAGS_algorithm == "quickturn")
            return thinkQuickTurn(frameId, f, seq);
        if (FLAGS_algorithm == "right")
            return thinkRight(frameId, f, seq);
        return thinkFlat(frameId, f, seq);
    }

private:
    DropDecision thinkFlat(int frameId, const CoreField& field, const KumipuyoSeq& seq) const
    {
        UNUSED_VARIABLE(frameId);

        pair<int, int> heights[] = {
            make_pair(field.height(1), 1),
            make_pair(field.height(6), 6),
            make_pair(field.height(2), 2),
            make_pair(field.height(5), 5),
            make_pair(field.height(4), 4),
            make_pair(field.height(3), 3),
        };

        sort(heights, heights + 6);
        for (int i = 0; i < 6; ++i) {
            int x = heights[i].second;
            if (!PuyoController::isReachable(field, Decision(x, 2)))
                continue;

            CoreField cf(field);
            cf.dropKumipuyo(Decision(x, 2), seq.get(0));
            if (!cf.rensaWillOccur())
                continue;

            return DropDecision(Decision(x, 2));
        }

        return DropDecision(Decision(3, 2));
    }

    DropDecision thinkPeak(int frameId, const CoreField& f, const KumipuyoSeq& seq) const
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

    DropDecision thinkQuickTurn(int frameId, const CoreField& f, const KumipuyoSeq& seq) const
    {
        UNUSED_VARIABLE(frameId);
        UNUSED_VARIABLE(seq);

        if (f.height(4) <= 10)
            return DropDecision(Decision(4, 2));
        if (f.height(4) == 11)
            return DropDecision(Decision(4, 1));
        if (f.height(2) <= 10)
            return DropDecision(Decision(2, 2));
        if (f.height(2) == 11)
            return DropDecision(Decision(2, 3));

        if (f.height(2) == 12 && f.height(4) == 12) {
            if (f.height(5) <= 10)
                return DropDecision(Decision(5, 2));
            if (f.height(5) == 11 || f.height(5) == 12)
                return DropDecision(Decision(5, 1));
            if (f.height(5) == 13)
                return DropDecision(Decision(4, 3));
            return DropDecision(Decision(4, 1));
        }

        if (f.height(2) == 12 && f.height(4) == 13)
            return DropDecision(Decision(2, 3));

        return DropDecision(Decision(3, 2));
    }

    DropDecision thinkRight(int frameId, const CoreField& f, const KumipuyoSeq& seq) const
    {
        UNUSED_VARIABLE(frameId);
        UNUSED_VARIABLE(seq);

        if (f.height(6) <= 10)
            return DropDecision(Decision(6, 2));
        if (f.height(5) <= 10)
            return DropDecision(Decision(5, 2));
        if (f.height(4) <= 10)
            return DropDecision(Decision(4, 2));
        return DropDecision(Decision(3, 2));
    }

    DropDecision thinkNohoho(int frameId, const CoreField& f, const KumipuyoSeq& seq) const
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

    ControlTesterAI(argc, argv).runLoop();
    return 0;
}
