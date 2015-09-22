#ifndef CPU_MAYAH_DECISION_PLANNER_H_
#define CPU_MAYAH_DECISION_PLANNER_H_

// TODO(mayah): Move this to core/algorithm.

#include <vector>

#include "base/executor.h"
#include "base/wait_group.h"
#include "core/plan/plan.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq.h"
#include "core/player_state.h"
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

    void iterate(int frameId, const CoreField& originalField, const KumipuyoSeq& kumipuyoSeq,
                 const PlayerState& me, const PlayerState& enemy, int maxDepth);

private:
    void iterateRest(int initialFrameId,
                     const CoreField& currentField,
                     const KumipuyoSeq& kumipuyoSeq,
                     const std::vector<Decision>& currentDecisions,
                     int currentNumChigiri,
                     int currentTotalFrames,
                     int currentDepth,
                     int maxDepth,
                     int fallenOjama,
                     int fixedOjama,
                     int pendingOjama,
                     int ojamaCommittingFrameId,
                     bool hasZenkeshi,
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

inline int updateOjama(int currentFrameId, int generatedOjama, int* fixedOjama, int* pendingOjama, int* ojamaCommittingFrameId)
{
    if (generatedOjama > 0) {
        if (*pendingOjama > generatedOjama) {
            *pendingOjama -= generatedOjama;
            generatedOjama = 0;
        } else {
            generatedOjama -= *pendingOjama;
            *pendingOjama = 0;
        }

        if (*fixedOjama > generatedOjama) {
            *fixedOjama -= generatedOjama;
            generatedOjama = 0;
        } else {
            generatedOjama -= *fixedOjama;
            *fixedOjama = 0;
        }
    }

    if (*ojamaCommittingFrameId == 0)
        return 0;

    if (*ojamaCommittingFrameId <= currentFrameId) {
        *fixedOjama += *pendingOjama;
        *pendingOjama = 0;
        *ojamaCommittingFrameId = 0;
    }

    int count = std::min(*fixedOjama, 30);
    *fixedOjama -= count;

    return count;
}

// Returns the number of frames for ojama falling.
inline int fallOjama(CoreField* cf, int ojamaCount)
{
    int lines = (ojamaCount + 2) / 6;
    if (lines > 5)
        lines = 5;
    return cf->fallOjama(lines);
}

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

    DCHECK(isNormalColor(kumipuyo.axis)) << kumipuyo.axis;
    DCHECK(isNormalColor(kumipuyo.child)) << kumipuyo.child;

    // Since copying CoreField is not so fast, we'd like to skip copying as many as possible.
    int numDecisions = kumipuyo.axis == kumipuyo.child ? 11 : 22;
    const Decision* decisionsHead = DECISIONS;

    // When decisions are specified, we consider only such decision.
    if (static_cast<size_t>(currentDepth) < decisions_.size()) {
        numDecisions = 1;
        decisionsHead = &decisions_[currentDepth];
    }

    for (int i = 0; i < numDecisions; ++i) {
        const Decision& decision = decisionsHead[i];

        if (!PuyoController::isReachable(currentField, decision))
            continue;

        CoreField nextField(currentField);
        if (!nextField.dropKumipuyo(decision, kumipuyo))
            continue;

        bool isChigiri = currentField.isChigiriDecision(decision);
        int dropFrames = currentField.framesToDropNext(decision);

        callback(std::move(nextField), decision, isChigiri, dropFrames);
    }
}


template<typename MidEvaluationResult>
void DecisionPlanner<MidEvaluationResult>::iterateRest(int initialFrameId,
                                                       const CoreField& currentField,
                                                       const KumipuyoSeq& kumipuyoSeq,
                                                       const std::vector<Decision>& currentDecisions,
                                                       int currentNumChigiri,
                                                       int currentTotalFrames,
                                                       int currentDepth,
                                                       int maxDepth,
                                                       int fallenOjama,
                                                       int fixedOjama,
                                                       int pendingOjama,
                                                       int ojamaCommittingFrameId,
                                                       bool hasZenkeshi,
                                                       const MidEvaluationResult& midEvaluationResult,
                                                       WaitGroup* wg)
{
    auto f = [&](CoreField&& fieldAfterDecision, const Decision& decision, bool isChigiri, int dropFrames) {
        std::vector<Decision> decisions(currentDecisions);
        decisions.push_back(decision);

        int newFixedOjama = fixedOjama;
        int newPendingOjama = pendingOjama;
        int newOjamaCommittingFrameId = ojamaCommittingFrameId;
        bool newHasZenkeshi = hasZenkeshi;

        int numChigiri = currentNumChigiri + (isChigiri ? 1 : 0);
        int frameIdToIgnite = initialFrameId + currentTotalFrames;
        // int framesToIgnite = currentTotalFrames;

        // --- When rensa will occur.
        if (fieldAfterDecision.rensaWillOccur()) {
            RensaResult rensaResult = fieldAfterDecision.simulate();
            int generatedOjama = rensaResult.score / 70 + (newHasZenkeshi ? 30 : 0);
            newHasZenkeshi = false;
            int newFallenOjama = updateOjama(frameIdToIgnite, generatedOjama, &newFixedOjama, &newPendingOjama, &newOjamaCommittingFrameId);
            int ojamaDroppingFrames = fallOjama(&fieldAfterDecision, newFallenOjama);
            parallelEval(currentDepth, RefPlan(fieldAfterDecision, decisions, rensaResult, numChigiri, currentTotalFrames, dropFrames + ojamaDroppingFrames,
                                               newFallenOjama + fallenOjama, newFixedOjama, newPendingOjama, newOjamaCommittingFrameId, newHasZenkeshi),
                         midEvaluationResult, wg);
            return;
        }

        // --- When rensa won't occur.
        int ojamaCount = updateOjama(frameIdToIgnite, 0, &newFixedOjama, &newPendingOjama, &newOjamaCommittingFrameId);
        int ojamaDroppingFrames = fallOjama(&fieldAfterDecision, ojamaCount);

        if (fieldAfterDecision.color(3, 12) != PuyoColor::EMPTY)
            return;

        if (currentDepth + 1 == maxDepth) {
            parallelEval(currentDepth,
                         RefPlan(fieldAfterDecision, decisions, RensaResult(), numChigiri, currentTotalFrames, dropFrames + ojamaDroppingFrames,
                                 ojamaCount + fallenOjama, newFixedOjama, newPendingOjama, newOjamaCommittingFrameId, newHasZenkeshi),
                         midEvaluationResult, wg);
            return;
        }

        int totalFrames = currentTotalFrames + dropFrames + ojamaDroppingFrames;
        if (executor_ && currentDepth <= 1) {
            wg->add(1);
            executor_->submit([=]() {
                iterateRest(initialFrameId, fieldAfterDecision, kumipuyoSeq, decisions, numChigiri, totalFrames, currentDepth + 1, maxDepth,
                            fallenOjama + ojamaCount,
                            newFixedOjama, newPendingOjama, newOjamaCommittingFrameId, newHasZenkeshi, midEvaluationResult, wg);

                wg->done();
            });
        } else {
            iterateRest(initialFrameId, fieldAfterDecision, kumipuyoSeq, decisions, numChigiri, totalFrames, currentDepth + 1, maxDepth,
                        fallenOjama + ojamaCount, newFixedOjama, newPendingOjama, newOjamaCommittingFrameId, newHasZenkeshi, midEvaluationResult, wg);
        }
    };

    iterateKumipuyoDrop(currentDepth, currentField, kumipuyoSeq.get(currentDepth), f);
}

template <typename MidEvaluationResult>
void DecisionPlanner<MidEvaluationResult>::iterate(int initialFrameId,
                                                   const CoreField& originalField,
                                                   const KumipuyoSeq& kumipuyoSeq,
                                                   const PlayerState& me,
                                                   const PlayerState& enemy,
                                                   int maxDepth)
{
    DCHECK(maxDepth >= 2);
    DCHECK(kumipuyoSeq.size() >= maxDepth);

    WaitGroup wg;

    auto f = [&](const CoreField& fieldAfterDecision, const Decision& decision, bool isChigiri, int dropFrames) {
        int fixedOjama = me.fixedOjama;
        int pendingOjama = me.pendingOjama;
        // TODO(mayah): Is it good to add ongoing ojama as pending ojama?
        // Add as pending ojama.
        pendingOjama += (enemy.unusedScore + enemy.currentRensaResult.score) / 70;
        pendingOjama -= enemy.fixedOjama + enemy.pendingOjama;
        if (pendingOjama < 0)
            pendingOjama = 0;

        int ojamaCommittingFrameId = enemy.isRensaOngoing() ? enemy.rensaFinishingFrameId() : 0;
        bool hasZenkeshi = me.hasZenkeshi;

        std::vector<Decision> decisions { decision };

        int numChigiri = isChigiri ? 1 : 0;

        if (fieldAfterDecision.rensaWillOccur()) {
            // When rensa will occur.
            CoreField cf(fieldAfterDecision);
            RensaResult rensaResult(cf.simulate());

            int generatedOjama = rensaResult.score / 70 + (hasZenkeshi ? 30 : 0);
            hasZenkeshi = false;
            int currentFrameId = initialFrameId + dropFrames + rensaResult.frames;
            int ojamaCount = updateOjama(currentFrameId, generatedOjama, &fixedOjama, &pendingOjama, &ojamaCommittingFrameId);
            int ojamaDroppingFrames = fallOjama(&cf, ojamaCount);

            parallelEval(0, RefPlan(cf, decisions, rensaResult, numChigiri, 0, dropFrames + ojamaDroppingFrames,
                                    ojamaCount, fixedOjama, pendingOjama, ojamaCommittingFrameId, hasZenkeshi),
                         MidEvaluationResult(), &wg);

            MidEvaluationResult midEvaluationResult =
                midEval_(RefPlan(cf, decisions, rensaResult, numChigiri, 0, dropFrames + ojamaDroppingFrames,
                                 ojamaCount, fixedOjama, pendingOjama, ojamaCommittingFrameId, hasZenkeshi));
            iterateRest(initialFrameId, cf, kumipuyoSeq, decisions, numChigiri, rensaResult.frames + dropFrames + ojamaDroppingFrames,
                        1, maxDepth, ojamaCount, fixedOjama, pendingOjama, ojamaCommittingFrameId, hasZenkeshi, midEvaluationResult, &wg);

            decisions.pop_back();
            return;
        }

        // When rensa doesn't occur.
        CoreField cf(fieldAfterDecision);
        int currentFrameId = initialFrameId + dropFrames;
        int ojamaCount = updateOjama(currentFrameId, 0, &fixedOjama, &pendingOjama, &ojamaCommittingFrameId);
        int ojamaDroppingFrames = fallOjama(&cf, ojamaCount);

        if (cf.color(3, 12) != PuyoColor::EMPTY) {
            decisions.pop_back();
            return;
        }

        MidEvaluationResult midEvaluationResult =
            midEval_(RefPlan(cf, decisions, RensaResult(), numChigiri, 0, dropFrames + ojamaDroppingFrames,
                             ojamaCount, fixedOjama, pendingOjama, ojamaCommittingFrameId, me.hasZenkeshi));

        iterateRest(initialFrameId, cf, kumipuyoSeq, decisions, numChigiri, dropFrames + ojamaDroppingFrames, 1, maxDepth,
                    ojamaCount, fixedOjama, pendingOjama, ojamaCommittingFrameId, hasZenkeshi, midEvaluationResult, &wg);

    };

    iterateKumipuyoDrop(0, originalField, kumipuyoSeq.get(0), f);
    wg.waitUntilDone();
}

template<typename MidEvaluationResult>
void DecisionPlanner<MidEvaluationResult>::parallelEval(int currentDepth, const RefPlan& refPlan,
                                                        const MidEvaluationResult& midEvaluationResult, WaitGroup* wg)
{
    // We only submit a task to executor when currentDepth <= 1. (current + next).
    // If we submit a task for currentDepth == 2, the number of task is too much, and overhead is high.
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

#endif // CPU_MAYAH_DECISION_PLANNER_H_
