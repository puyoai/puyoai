#ifndef CORE_PLAN_PLAN_H_
#define CORE_PLAN_PLAN_H_

#include <functional>
#include <string>
#include <vector>

#include "base/noncopyable.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "core/rensa_result.h"

class KumipuyoSeq;
class RefPlan;

class Plan {
public:
    // Event which may happen during the iteration.
    // Extend Type if you want to use more types of events.
    struct Event {
        enum class Type {
            FALL_OJAMA_ROWS,
        };

        static Event fallOjamaRowsEvent(int frame, int rows)
        {
            return Event { Type::FALL_OJAMA_ROWS, frame, rows };
        }

        Type type;

        // This figures when this event will happen. 0 figures the frameId when a API is called.
        int frames;

        // Meaning of this variable depends on which type of event.
        int value;
    };

    Plan() {}
    Plan(const CoreField& field, const std::vector<Decision>& decisions,
         const RensaResult& rensaResult, int numChigiri, int framesToIgnite, int lastDropFrames,
         int fallenOjama, int fixedOjama, int pendingOjama, int ojamaCommittingFrameId, bool hasZenkeshi) :
        field_(field), decisions_(decisions), rensaResult_(rensaResult),
        numChigiri_(numChigiri), framesToIgnite_(framesToIgnite), lastDropFrames_(lastDropFrames),
        fallenOjama_(fallenOjama), fixedOjama_(fixedOjama), pendingOjama_(pendingOjama),
        ojamaCommittingFrameId_(ojamaCommittingFrameId), hasZenkeshi_(hasZenkeshi)
    {
    }

    typedef std::function<void (const RefPlan&)> IterationCallback;
    // if |kumipuyos.size()| < |depth|, we will add extra kumipuyo.
    static void iterateAvailablePlans(const CoreField&, const KumipuyoSeq&, int depth, const IterationCallback&);
    // We assume events are sorted in increasing order of frames.
    static void iterateAvailablePlansWithEvents(const CoreField&, const KumipuyoSeq&, int depth,
                                                const std::vector<Event>& events, const IterationCallback&);

    typedef std::function<void (const CoreField&, const std::vector<Decision>&,
                                int numChigiri, int framesToIgnite, int lastDropFrames, bool shouldFire)> RensaIterationCallback;
    static void iterateAvailablePlansWithoutFiring(const CoreField&, const KumipuyoSeq&, int depth, const RensaIterationCallback&);
    // We assume events are sorted in increasing order of frames.
    static void iterateAvailablePlansWithoutFiringWithEvents(const CoreField&, const KumipuyoSeq&, int depth,
                                                             const std::vector<Event>& events, const RensaIterationCallback&);

    const CoreField& field() const { return field_; }

    const Decision& firstDecision() const { return decisions_[0]; }
    const Decision& decision(int nth) const { return decisions_[nth]; }
    const std::vector<Decision>& decisions() const { return decisions_; }

    const RensaResult& rensaResult() const { return rensaResult_; }
    int framesToIgnite() const { return framesToIgnite_; }
    int lastDropFrames() const { return lastDropFrames_; }

    int score() const { return rensaResult_.score; }
    int chains() const { return rensaResult_.chains; }

    int numChigiri() const { return numChigiri_; }
    int totalFrames() const { return framesToIgnite_ + lastDropFrames_ + rensaResult_.frames; }

    bool isRensaPlan() const { return rensaResult_.chains > 0; }

    int fallenOjama() const { return fallenOjama_; }
    int pendingOjama() const { return pendingOjama_; }
    int fixedOjama() const { return fixedOjama_; }
    int totalOjama() const { return pendingOjama_ + fixedOjama_; }
    int ojamaCommittingFrameId() const { return ojamaCommittingFrameId_; }
    bool hasZenkeshi() const { return hasZenkeshi_; }

    std::string decisionText() const;

    friend bool operator==(const Plan& lhs, const Plan& rhs);
private:
    CoreField field_;      // Future field (after the rensa has been finished).
    std::vector<Decision> decisions_;
    RensaResult rensaResult_;
    int numChigiri_ = 0;
    int framesToIgnite_ = 0;
    int lastDropFrames_ = 0;
    int fallenOjama_ = 0;
    int fixedOjama_ = 0;
    int pendingOjama_ = 0;
    int ojamaCommittingFrameId_ = 0;
    bool hasZenkeshi_ = 0;
};

// RefPlan is almost same as plan, but holding references so that field copy will not occur.
// You can convert this to Plan with calling toPlan().
class RefPlan : noncopyable {
public:
    explicit RefPlan(const Plan& plan) :
        field_(plan.field()), decisions_(plan.decisions()), rensaResult_(plan.rensaResult()),
        numChigiri_(plan.numChigiri()), framesToIgnite_(plan.framesToIgnite()), lastDropFrames_(plan.lastDropFrames()),
        fallenOjama_(plan.fallenOjama()), fixedOjama_(plan.fixedOjama()), pendingOjama_(plan.pendingOjama()),
        ojamaCommittingFrameId_(plan.ojamaCommittingFrameId()), hasZenkeshi_(plan.hasZenkeshi())
    {
    }

    RefPlan(const CoreField& field, const std::vector<Decision>& decisions,
            const RensaResult& rensaResult, int numChigiri, int framesToIgnite, int lastDropFrames,
            int fallenOjama, int fixedOjama, int pendingOjama, int ojamaCommittingFrameId, bool hasZenkeshi) :
        field_(field), decisions_(decisions), rensaResult_(rensaResult),
        numChigiri_(numChigiri), framesToIgnite_(framesToIgnite), lastDropFrames_(lastDropFrames),
        fallenOjama_(fallenOjama), fixedOjama_(fixedOjama), pendingOjama_(pendingOjama),
        ojamaCommittingFrameId_(ojamaCommittingFrameId), hasZenkeshi_(hasZenkeshi)
    {
    }

    const CoreField& field() const { return field_; }
    const std::vector<Decision>& decisions() const { return decisions_; }
    const Decision& decision(int nth) const { return decisions_[nth]; }
    const Decision& firstDecision() const { return decision(0); }
    const RensaResult& rensaResult() const { return rensaResult_; }

    int chains() const { return rensaResult_.chains; }
    int score() const { return rensaResult_.score; }

    // framesToIgnite returns how many frames are required just before the last hand.
    int framesToIgnite() const { return framesToIgnite_; }
    int lastDropFrames() const { return lastDropFrames_; }
    int totalFrames() const { return framesToIgnite_ + lastDropFrames_ + rensaResult_.frames; }
    int numChigiri() const { return numChigiri_; }

    bool isRensaPlan() const { return rensaResult_.chains > 0; }

    int fallenOjama() const { return fallenOjama_; }
    int pendingOjama() const { return pendingOjama_; }
    int fixedOjama() const { return fixedOjama_; }
    int totalOjama() const { return pendingOjama_ + fixedOjama_; }
    int ojamaCommittingFrameId() const { return ojamaCommittingFrameId_; }
    bool hasZenkeshi() const { return hasZenkeshi_; }

    Plan toPlan() const { return Plan(field_, decisions_, rensaResult_, numChigiri_, framesToIgnite_, lastDropFrames_,
                                      fallenOjama_, pendingOjama_, fixedOjama_, ojamaCommittingFrameId_, hasZenkeshi_); }

    std::string decisionText() const;

private:
    const CoreField& field_;
    const std::vector<Decision>& decisions_;
    const RensaResult& rensaResult_;
    int numChigiri_;
    int framesToIgnite_;
    int lastDropFrames_;
    int fallenOjama_;
    int fixedOjama_;
    int pendingOjama_;
    int ojamaCommittingFrameId_;
    bool hasZenkeshi_;
};

#endif // CORE_PLAN_PLAN_H_
