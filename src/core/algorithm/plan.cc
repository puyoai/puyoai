#include "core/algorithm/plan.h"

#include <iostream>
#include <sstream>

#include "core/ctrl.h"
#include "core/kumipuyo.h"

using namespace std;

static const Decision DECISIONS[] = {
    Decision(2, 3), Decision(3, 3), Decision(3, 1), Decision(4, 1),
    Decision(5, 1), Decision(1, 2), Decision(2, 2), Decision(3, 2),
    Decision(4, 2), Decision(5, 2), Decision(6, 2), Decision(1, 1),
    Decision(2, 1), Decision(4, 3), Decision(5, 3), Decision(6, 3),
    Decision(1, 0), Decision(2, 0), Decision(3, 0), Decision(4, 0),
    Decision(5, 0), Decision(6, 0),
};

static const Kumipuyo ALL_KUMIPUYO_KINDS[] = {
    Kumipuyo(PuyoColor::RED, PuyoColor::RED),
    Kumipuyo(PuyoColor::RED, PuyoColor::BLUE),
    Kumipuyo(PuyoColor::RED, PuyoColor::YELLOW),
    Kumipuyo(PuyoColor::RED, PuyoColor::GREEN),
    Kumipuyo(PuyoColor::BLUE, PuyoColor::BLUE),
    Kumipuyo(PuyoColor::BLUE, PuyoColor::YELLOW),
    Kumipuyo(PuyoColor::BLUE, PuyoColor::GREEN),
    Kumipuyo(PuyoColor::YELLOW, PuyoColor::YELLOW),
    Kumipuyo(PuyoColor::YELLOW, PuyoColor::GREEN),
    Kumipuyo(PuyoColor::GREEN, PuyoColor::GREEN),
};

std::string Plan::decisionText() const
{
    std::ostringstream ss;
    for (size_t i = 0; i < decisions().size(); ++i) {
        if (i)
            ss << '-';
        ss << decision(i).toString();
    }

    return ss.str();
}

std::string RefPlan::decisionText() const
{
    std::ostringstream ss;
    for (size_t i = 0; i < decisions().size(); ++i) {
        if (i)
            ss << '-';
        ss << decisions()[i].toString();
    }

    return ss.str();
}

static void iterateAvailablePlansInternal(const CoreField& field,
                                          const KumipuyoSeq& kumipuyoSeq,
                                          std::vector<Decision>& decisions,
                                          int currentDepth,
                                          int maxDepth,
                                          int totalFrames,
                                          Plan::IterationCallback callback)
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

    // Since copying Field is slow, we'd like to skip copying as many as possible.
    CoreField nextField(field);
    bool needsNextFieldRefresh = false;

    for (int i = 0; i < n; ++i) {
        const Kumipuyo& kumipuyo = ptr[i];
        int num_decisions = (kumipuyo.axis == kumipuyo.child) ? 11 : 22;

        for (int i = 0; i < num_decisions; i++) {
            if (needsNextFieldRefresh) {
                needsNextFieldRefresh = false;
                nextField = field;
            }

            const Decision& decision = DECISIONS[i];
            if (!Ctrl::isReachable(field, decision))
                continue;

            if (!nextField.dropKumipuyo(decision, kumipuyo))
                continue;

            int dropFrames = nextField.framesToDropNext(decision);
            // CoreField::simulate is slow. So, if the last decision does not invoke any rensa,
            // we'd like to skip simulate.
            RensaResult rensaResult = nextField.rensaWillOccurWhenLastDecisionIs(decision) ? nextField.simulate() : RensaResult();
            if (rensaResult.chains > 0)
                needsNextFieldRefresh = true;
            if (nextField.color(3, 12) != PuyoColor::EMPTY) {
                if (rensaResult.chains == 0)
                    nextField.undoKumipuyo(decision);
                continue;
            }

            decisions.push_back(decision);
            if (currentDepth + 1 == maxDepth || rensaResult.chains > 0) {
                callback(RefPlan(nextField, decisions, rensaResult, totalFrames));
            } else {
                iterateAvailablePlansInternal(nextField, kumipuyoSeq, decisions, currentDepth + 1, maxDepth,
                                              totalFrames + dropFrames, callback);
            }
            decisions.pop_back();

            if (rensaResult.chains == 0)
                nextField.undoKumipuyo(decision);
        }
    }
}

// static
void Plan::iterateAvailablePlans(const CoreField& field,
                                 const KumipuyoSeq& kumipuyoSeq,
                                 int maxDepth,
                                 Plan::IterationCallback callback)
{
    std::vector<Decision> decisions;
    decisions.reserve(maxDepth);
    iterateAvailablePlansInternal(field, kumipuyoSeq, decisions, 0, maxDepth, 0, callback);
}
