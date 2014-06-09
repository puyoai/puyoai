#include "core/algorithm/plan.h"

#include <sstream>
#include "core/ctrl.h"
#include "core/kumipuyo.h"

using namespace std;

static const Decision DECISIONS[22] = {
    Decision(2, 3), Decision(3, 3), Decision(3, 1), Decision(4, 1), Decision(5, 1),
    Decision(1, 2), Decision(2, 2), Decision(3, 2), Decision(4, 2), Decision(5, 2), Decision(6, 2),
    Decision(1, 1), Decision(2, 1), Decision(4, 3), Decision(5, 3), Decision(6, 3),
    Decision(1, 0), Decision(2, 0), Decision(3, 0), Decision(4, 0), Decision(5, 0), Decision(6, 0),
};

static const Kumipuyo ALL_KUMIPUYO_KINDS[10] = {
    Kumipuyo(PuyoColor::RED, PuyoColor::RED), Kumipuyo(PuyoColor::RED, PuyoColor::BLUE), Kumipuyo(PuyoColor::RED, PuyoColor::YELLOW), Kumipuyo(PuyoColor::RED, PuyoColor::GREEN),
    Kumipuyo(PuyoColor::BLUE, PuyoColor::BLUE), Kumipuyo(PuyoColor::BLUE, PuyoColor::YELLOW), Kumipuyo(PuyoColor::BLUE, PuyoColor::GREEN),
    Kumipuyo(PuyoColor::YELLOW, PuyoColor::YELLOW), Kumipuyo(PuyoColor::YELLOW, PuyoColor::GREEN),
    Kumipuyo(PuyoColor::GREEN, PuyoColor::GREEN),
};

std::string Plan::decisionText() const
{
    ostringstream ss;

    for (int i = 0; i < decisions().size(); ++i) {
        if (i)
            ss << '-';
        ss << decision(i).toString();
    }

    return ss.str();
}

// static
vector<Plan> Plan::findAvailablePlans(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    DCHECK(1 <= kumipuyoSeq.size() && kumipuyoSeq.size() <= 3);

    vector<Plan> plans;
    plans.reserve(22 + 22*22 + 22*22*22);

    iterateAvailablePlans(field, kumipuyoSeq, kumipuyoSeq.size(), [&plans](const RefPlan& ref){
        plans.push_back(ref.toPlan());
    });

    return plans;
}

static void iterateAvailablePlansInternal(const CoreField& field, const KumipuyoSeq& kumipuyoSeq,
                                          vector<Decision>& decisions, int currentDepth, int maxDepth,
                                          int totalFrames, Plan::IterationCallback callback)
{
    const Kumipuyo* ptr;
    int n;

    Kumipuyo tmp;
    if (currentDepth < kumipuyoSeq.size()) {
        tmp = kumipuyoSeq.get(currentDepth);
        ptr = &tmp;
        n = 1;
    } else {
        ptr = ALL_KUMIPUYO_KINDS;
        n = 10;
    }

    for (int i = 0; i < n; ++i) {
        const Kumipuyo& kumipuyo = ptr[i];
        int num_decisions = (kumipuyo.axis == kumipuyo.child) ? 11 : 22;

        for (int i = 0; i < num_decisions; i++) {
            const Decision& decision = DECISIONS[i];
            if (!Ctrl::isReachable(field, decision))
                continue;

            CoreField nextField(field);
            if (!nextField.dropKumipuyo(decision, kumipuyo))
                continue;

            int dropFrames = nextField.framesToDropNext(decision);
            BasicRensaResult rensaResult = nextField.simulate();
            if (nextField.color(3, 12) != PuyoColor::EMPTY)
                continue;

            decisions.push_back(decision);
            if (currentDepth + 1 == maxDepth || rensaResult.chains > 0) {
                callback(RefPlan(nextField, decisions, rensaResult, totalFrames));
            } else {
                iterateAvailablePlansInternal(nextField, kumipuyoSeq, decisions,
                                              currentDepth + 1, maxDepth,
                                              totalFrames + dropFrames, callback);
            }
            decisions.pop_back();
        }
    }
}

// static
void Plan::iterateAvailablePlans(const CoreField& field, const KumipuyoSeq& kumipuyoSeq, int maxDepth,
                                 Plan::IterationCallback callback)
{
    vector<Decision> decisions;
    decisions.reserve(maxDepth);
    iterateAvailablePlansInternal(field, kumipuyoSeq, decisions, 0, maxDepth, 0, callback);
}
