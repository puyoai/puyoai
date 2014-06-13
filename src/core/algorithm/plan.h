#ifndef CORE_ALGORITHM_PLAN_H_
#define CORE_ALGORITHM_PLAN_H_

#include <functional>
#include <string>
#include <vector>

#include "base/base.h"
#include "core/decision.h"
#include "core/field/core_field.h"
#include "core/field/rensa_result.h"

class KumipuyoSeq;
class RefPlan;

class Plan {
public:
    Plan(const CoreField& field, const std::vector<Decision>& decisions,
         const BasicRensaResult& rensaResult, int initiatingFrames) :
        field_(field), decisions_(decisions), rensaResult_(rensaResult),
        initiatingFrames_(initiatingFrames)
    {
    }

    typedef std::function<void (const RefPlan&)> IterationCallback;
    static std::vector<Plan> findAvailablePlans(const CoreField&, const KumipuyoSeq&) DEPRECATED_MSG("use iterateAvailablePlans instead.");
    // if |kumipuyos.size()| < |depth|, we will add extra kumipuyo.
    static void iterateAvailablePlans(const CoreField&, const KumipuyoSeq&, int depth, IterationCallback);

    const CoreField& field() const { return field_; }

    const Decision& firstDecision() const { return decisions_[0]; }
    const Decision& decision(int nth) const { return decisions_[nth]; }
    const std::vector<Decision>& decisions() const { return decisions_; }

    const BasicRensaResult& rensaResult() const { return rensaResult_; }
    int initiatingFrames() const { return initiatingFrames_; }

    int score() const { return rensaResult_.score; }
    int chains() const { return rensaResult_.chains; }

    int totalFrames() const { return initiatingFrames_ + rensaResult_.frames; }

    bool isRensaPlan() const { return rensaResult_.chains > 0; }

    std::string decisionText() const;

private:
    CoreField field_;      // Future field (after the rensa has been finished).
    std::vector<Decision> decisions_;
    BasicRensaResult rensaResult_;
    int initiatingFrames_;
};

// RefPlan is almost same as plan, but holding references so that field copy will not occur.
// You can convert this to Plan with calling toPlan().
class RefPlan : noncopyable {
public:
    RefPlan(const CoreField& field, const std::vector<Decision>& decisions,
            const BasicRensaResult& rensaResult, int initiatingFrames) :
        field_(field), decisions_(decisions), rensaResult_(rensaResult),
        initiatingFrames_(initiatingFrames)
    {
    }

    const CoreField& field() const { return field_; }
    const std::vector<Decision>& decisions() const { return decisions_; }
    const BasicRensaResult& rensaResult() const { return rensaResult_; }

    int chains() const { return rensaResult_.chains; }
    int score() const { return rensaResult_.score; }

    // initiatingFrames returns how many frames are required just before the last hand.
    int initiatingFrames() const { return initiatingFrames_; }
    int totalFrames() const { return initiatingFrames_ + rensaResult_.frames; }

    bool isRensaPlan() const { return rensaResult_.chains > 0; }

    Plan toPlan() const { return Plan(field_, decisions_, rensaResult_, initiatingFrames_); }

    std::string decisionText() const;

private:
    const CoreField& field_;
    const std::vector<Decision>& decisions_;
    const BasicRensaResult& rensaResult_;
    int initiatingFrames_;
};

#endif
