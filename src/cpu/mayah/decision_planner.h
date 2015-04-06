#ifndef CPU_MAYAH_DECISION_PLANNER_H_
#define CPU_MAYAH_DECISION_PLANNER_H_

#include <vector>

#include "base/executor.h"
#include "base/wait_group.h"
#include "core/algorithm/plan.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq.h"
#include "core/puyo_controller.h"

template<typename MidEvaluationResult>
class DecisionPlanner {
public:
    typedef std::function<MidEvaluationResult (const RefPlan&)> MidEvaluationCallback;
    typedef std::function<void (const RefPlan&, const MidEvaluationResult&)> EvaluationCallback;

    DecisionPlanner(Executor* executor, MidEvaluationCallback midEval, EvaluationCallback eval) :
        executor_(executor),
        midEval_(std::move(midEval)),
        eval_(std::move(eval))
    {
    }

    DecisionPlanner(MidEvaluationCallback midEval, EvaluationCallback eval) :
        executor_(nullptr),
        midEval_(std::move(midEval)),
        eval_(std::move(eval))
    {
    }

    // When decision sequence is specified, we consider only this decision sequence.
    void setSpecifiedDecisions(const std::vector<Decision>& decisions) { decisions_ = decisions; }

    void iterate(const CoreField&, const KumipuyoSeq&, int maxDepth);

private:
    void iterateRest(const CoreField& currentField,
                     const KumipuyoSeq& kumipuyoSeq,
                     const std::vector<Decision>& currentDecisions,
                     int currentNumChigiri,
                     int currentTotalFrames,
                     int currentDepth,
                     int maxDepth,
                     const MidEvaluationResult& midEvaluationResult,
                     WaitGroup* wg);

    void parallelEval(int currentDepth, const RefPlan& plan, const MidEvaluationResult& midEvaluationResult, WaitGroup* wg);

     // callback: void (const CoreField&, const Decision&, bool isChigiri, int dropFrames);
    template<typename Callback>
    void iterateKumipuyoDrop(int currentDepth, const CoreField& currentField, const Kumipuyo& kumipuyo, Callback callback);

    Executor* executor_;
    std::vector<Decision> decisions_;
    MidEvaluationCallback midEval_;
    EvaluationCallback eval_;
};

// ----------------------------------------------------------------------

template<typename MidEvaluationResult>
template<typename Callback>
void DecisionPlanner<MidEvaluationResult>::iterateKumipuyoDrop(int currentDepth,
                                                               const CoreField& currentField,
                                                               const Kumipuyo& kumipuyo,
                                                               Callback callback)
{
    static const Decision DECISIONS[] = {
        Decision(2, 3), Decision(3, 3), Decision(3, 1), Decision(4, 1),
        Decision(5, 1), Decision(1, 2), Decision(2, 2), Decision(3, 2),
        Decision(4, 2), Decision(5, 2), Decision(6, 2),
        Decision(1, 1), Decision(2, 1), Decision(4, 3), Decision(5, 3),
        Decision(6, 3), Decision(1, 0), Decision(2, 0), Decision(3, 0),
        Decision(4, 0), Decision(5, 0), Decision(6, 0),
    };

    // Since copying CoreField is not so fast, we'd like to skip copying as many as possible.
    CoreField nextField(currentField);
    int numDecisions = kumipuyo.axis == kumipuyo.child ? 11 : 22;
    const Decision* decisionsHead = DECISIONS;

    // When decisions are specified, we consider only such decision.
    if (static_cast<size_t>(currentDepth) < decisions_.size()) {
        numDecisions = 1;
        decisionsHead = &decisions_[currentDepth];
    }

    for (int i = 0; i < numDecisions; ++i) {
        DCHECK(nextField == currentField);
        const Decision& decision = decisionsHead[i];

        if (!PuyoController::isReachable(currentField, decision))
            continue;

        if (!nextField.dropKumipuyo(decision, kumipuyo))
            continue;

        bool isChigiri = currentField.isChigiriDecision(decision);
        int dropFrames = currentField.framesToDropNext(decision);

        callback(nextField, decision, isChigiri, dropFrames);
        nextField.undoKumipuyo(decision);
    }
}


template<typename MidEvaluationResult>
void DecisionPlanner<MidEvaluationResult>::iterateRest(const CoreField& currentField,
                                                       const KumipuyoSeq& kumipuyoSeq,
                                                       const std::vector<Decision>& currentDecisions,
                                                       int currentNumChigiri,
                                                       int currentTotalFrames,
                                                       int currentDepth,
                                                       int maxDepth,
                                                       const MidEvaluationResult& midEvaluationResult,
                                                       WaitGroup* wg)
{
    const CoreField::SimulationContext currentContext(CoreField::SimulationContext::fromField(currentField));
    auto f = [&](const CoreField& fieldAfterDecision, const Decision& decision, bool isChigiri, int dropFrames) {
        std::vector<Decision> decisions(currentDecisions);
        decisions.push_back(decision);

        int numChigiri = currentNumChigiri + (isChigiri ? 1 : 0);
        int framesToIgnite = currentTotalFrames;

        // --- When rensa will occur.
        if (fieldAfterDecision.rensaWillOccurWithContext(currentContext)) {
            CoreField cf(fieldAfterDecision);
            CoreField::SimulationContext context(currentContext);
            RensaResult rensaResult = cf.simulate(&context);
            parallelEval(currentDepth, RefPlan(cf, decisions, rensaResult, numChigiri, framesToIgnite, dropFrames), midEvaluationResult, wg);
            return;
        }

        // --- When rensa won't occur.
        if (fieldAfterDecision.color(3, 12) != PuyoColor::EMPTY)
            return;

        if (currentDepth + 1 == maxDepth) {
            parallelEval(currentDepth, RefPlan(fieldAfterDecision, decisions, RensaResult(), numChigiri, framesToIgnite, dropFrames),
                         midEvaluationResult, wg);
            return;
        }

        int totalFrames = currentTotalFrames + dropFrames;
        if (executor_ && currentDepth <= 1) {
            wg->add(1);
            executor_->submit([=]() {
                iterateRest(fieldAfterDecision, kumipuyoSeq, decisions, numChigiri, totalFrames,
                            currentDepth + 1, maxDepth, midEvaluationResult, wg);
                wg->done();
            });
        } else {
            iterateRest(fieldAfterDecision, kumipuyoSeq, decisions, numChigiri, totalFrames,
                        currentDepth + 1, maxDepth, midEvaluationResult, wg);
        }
    };

    iterateKumipuyoDrop(currentDepth, currentField, kumipuyoSeq.get(currentDepth), f);
}

template <typename MidEvaluationResult>
void DecisionPlanner<MidEvaluationResult>::iterate(const CoreField& originalField,
                                                   const KumipuyoSeq& kumipuyoSeq,
                                                   int maxDepth)
{
    DCHECK(maxDepth >= 2);
    DCHECK(kumipuyoSeq.size() >= maxDepth);

    WaitGroup wg;

    std::vector<Decision> decisions;
    decisions.reserve(maxDepth);

    CoreField::SimulationContext originalContext(CoreField::SimulationContext::fromField(originalField));
    auto f = [&](const CoreField& fieldAfterDecision, const Decision& decision, bool isChigiri, int dropFrames) {
        decisions.push_back(decision);

        int numChigiri = isChigiri ? 1 : 0;

        if (fieldAfterDecision.rensaWillOccurWithContext(originalContext)) {
            // When rensa will occur.

            CoreField cf(fieldAfterDecision);
            CoreField::SimulationContext context(originalContext);
            RensaResult rensaResult(cf.simulate(&context));
            parallelEval(0, RefPlan(cf, decisions, rensaResult, numChigiri, 0, dropFrames), MidEvaluationResult(), &wg);

            // Continue the evaluation if erased.
            MidEvaluationResult midEvaluationResult = midEval_(RefPlan(cf, decisions, rensaResult, numChigiri, 0, dropFrames));
            iterateRest(cf, kumipuyoSeq, decisions, numChigiri, rensaResult.frames + dropFrames, 1, maxDepth, midEvaluationResult, &wg);

            decisions.pop_back();
            return;
        }

        // When rensa doesn't occur.
        if (fieldAfterDecision.color(3, 12) != PuyoColor::EMPTY) {
            decisions.pop_back();
            return;
        }

        MidEvaluationResult midEvaluationResult = midEval_(RefPlan(fieldAfterDecision, decisions, RensaResult(), numChigiri, 0, dropFrames));
        iterateRest(fieldAfterDecision, kumipuyoSeq, decisions, numChigiri, dropFrames, 1, maxDepth, midEvaluationResult, &wg);
        decisions.pop_back();
    };

    iterateKumipuyoDrop(0, originalField, kumipuyoSeq.get(0), f);

    wg.waitUntilDone();
}

template<typename MidEvaluationResult>
void DecisionPlanner<MidEvaluationResult>::parallelEval(int currentDepth, const RefPlan& refPlan,
                                                        const MidEvaluationResult& midEvaluationResult, WaitGroup* wg)
{
    if (executor_ && currentDepth <= 1) {
        wg->add(1);
        Plan plan(refPlan.toPlan());
        executor_->submit([this, plan, midEvaluationResult, wg]() {
                this->eval_(RefPlan(plan), midEvaluationResult);
                wg->done();
        });
    } else {
        eval_(refPlan, midEvaluationResult);
    }
}

#endif
