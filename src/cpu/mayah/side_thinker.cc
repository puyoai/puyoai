#include "side_thinker.h"

#include "core/rensa/rensa_detector.h"
#include "core/plan/plan.h"

SideThinker::SideThinker()
{
}

DropDecision SideThinker::think(int /*frame_id*/, const CoreField& field, const KumipuyoSeq& seq,
                                const PlayerState& /*me*/, const PlayerState& /*enemy*/, bool /*fast*/) const
{
    Decision best_fire;
    int best_fire_score = 0;

    Decision best;
    int best_score = 100;

    Plan::iterateAvailablePlans(field, seq, 2, [&](const RefPlan& plan) {
        auto callback = [&](CoreField&& complemenedField, const ColumnPuyoList& cpl) {
            RensaResult result = complemenedField.simulate();
            if (result.chains != 2 || result.score < 1000)
                return;
            if (cpl.size() < best_score) {
                best_score = cpl.size();
                best = plan.firstDecision();
            }
        };

        if (plan.isRensaPlan()) {
            if (plan.chains() == 2 && plan.score() >= 1000 && plan.score() > best_fire_score) {
                best_fire = plan.firstDecision();
                best_fire_score = plan.score();
            }
        }

        RensaDetector::detectSideChain(field, RensaDetectorStrategy::defaultDropStrategy(), callback);
    });

    if (best_fire.isValid())
        return DropDecision(best_fire);
    if (best.isValid())
        return DropDecision(best);
    return DropDecision();
}
