#include "core/algorithm/plan.h"

#include <iostream>
#include <sstream>

#include "core/constant.h"
#include "core/kumipuyo_seq.h"
#include "core/puyo_controller.h"

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

// friend static
bool operator==(const Plan& lhs, const Plan& rhs)
{
    return lhs.field_ == rhs.field_ &&
        lhs.decisions_ == rhs.decisions_ &&
        lhs.rensaResult_ == rhs.rensaResult_ &&
        lhs.numChigiri_ == rhs.numChigiri_ &&
        lhs.framesToIgnite_ == rhs.framesToIgnite_ &&
        lhs.lastDropFrames_ == rhs.lastDropFrames_;
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

template<typename Callback>
void iterateAvailablePlansInternal(const CoreField& field,
                                   const KumipuyoSeq& kumipuyoSeq,
                                   std::vector<Decision>& decisions,
                                   int currentDepth,
                                   int maxDepth,
                                   int currentNumChigiri,
                                   int totalFrames,
                                   Callback callback)
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

    for (int i = 0; i < n; ++i) {
        const Kumipuyo& kumipuyo = ptr[i];
        int num_decisions = (kumipuyo.axis == kumipuyo.child) ? 11 : 22;

        for (int j = 0; j < num_decisions; j++) {
            DCHECK(nextField == field);

            const Decision& decision = DECISIONS[j];
            if (!PuyoController::isReachable(field, decision))
                continue;

            if (!nextField.dropKumipuyo(decision, kumipuyo))
                continue;

            bool isChigiri = field.isChigiriDecision(decision);
            int dropFrames = field.framesToDropNext(decision);

            // CoreField::simulate is slow. So, if the last decision does not invoke any rensa,
            // we'd like to skip simulate.
            // Even using simulateWhenLastDecisionIs, it looks checking rensaWillOccurWhenLastDecisionIs
            // is 7~8 % faster.
            if (nextField.rensaWillOccurWhenLastDecisionIs(decision)) {
                decisions.push_back(decision);
                callback(nextField, decisions, currentNumChigiri + isChigiri, totalFrames, dropFrames, true);
                decisions.pop_back();
                nextField.undoKumipuyo(decision);
            } else {
                if (nextField.color(3, 12) != PuyoColor::EMPTY) {
                    nextField.undoKumipuyo(decision);
                    continue;
                }

                decisions.push_back(decision);
                if (currentDepth + 1 == maxDepth) {
                    callback(nextField, decisions, currentNumChigiri + isChigiri, totalFrames, dropFrames, false);
                } else {
                    iterateAvailablePlansInternal(nextField, kumipuyoSeq, decisions, currentDepth + 1, maxDepth,
                                                  currentNumChigiri + isChigiri, totalFrames + dropFrames, callback);
                }
                decisions.pop_back();
                nextField.undoKumipuyo(decision);
            }
        }
    }
}

// static
void Plan::iterateAvailablePlans(const CoreField& field,
                                 const KumipuyoSeq& kumipuyoSeq,
                                 int maxDepth,
                                 const Plan::IterationCallback& callback)
{
    std::vector<Decision> decisions;
    decisions.reserve(maxDepth);

    auto f = [&callback](const CoreField& fieldBeforeRensa, const std::vector<Decision>& decisions,
                         int numChigiri, int framesToIgnite, int lastDropFrames, bool shouldFire) {
        DCHECK(!decisions.empty());

        if (shouldFire) {
            CoreField cf(fieldBeforeRensa);
            CoreField::SimulationContext context(CoreField::SimulationContext::fromLastDecision(cf, decisions.back()));
            RensaResult rensaResult = cf.simulate(&context);
            DCHECK_GT(rensaResult.chains, 0);
            if (cf.color(3, 12) == PuyoColor::EMPTY) {
                callback(RefPlan(cf, decisions, rensaResult, numChigiri, framesToIgnite, lastDropFrames));
            }
        } else {
            if (fieldBeforeRensa.color(3, 12) == PuyoColor::EMPTY) {
                RensaResult rensaResult;
                callback(RefPlan(fieldBeforeRensa, decisions, rensaResult, numChigiri, framesToIgnite, lastDropFrames));
            }
        }
    };

    iterateAvailablePlansInternal(field, kumipuyoSeq, decisions, 0, maxDepth, 0, 0, f);
}

// static
void Plan::iterateAvailablePlansWithoutFiring(const CoreField& field,
                                              const KumipuyoSeq& kumipuyoSeq,
                                              int maxDepth,
                                              const Plan::RensaIterationCallback& callback)
{
    std::vector<Decision> decisions;
    decisions.reserve(maxDepth);
    iterateAvailablePlansInternal(field, kumipuyoSeq, decisions, 0, maxDepth, 0, 0, callback);
}
