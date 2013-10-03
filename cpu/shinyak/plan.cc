#include "plan.h"

#include <sstream>
#include "core/ctrl.h"
#include "rensa_result.h"

using namespace std;

std::string Plan::decisionText() const
{
    ostringstream ss;
    
    for (int i = 0; i < numDecisions(); ++i) {
        if (i)
            ss << '-';
        ss << decision(i).toString();
    }

    return ss.str();
}

static void findAvailablePlansInternal(const Field& field,
                                const Plan* previousPlan,
                                int restDepth, int nth,
                                const vector<KumiPuyo>& kumiPuyos, vector<Plan>& plans)
{
    static const Decision decisions[22] = {
        Decision(2, 3), Decision(3, 3), Decision(3, 1), Decision(4, 1), Decision(5, 1), 
        Decision(1, 2), Decision(2, 2), Decision(3, 2), Decision(4, 2), Decision(5, 2), Decision(6, 2),         
        Decision(1, 1), Decision(2, 1), Decision(4, 3), Decision(5, 3), Decision(6, 3), 
        Decision(1, 0), Decision(2, 0), Decision(3, 0), Decision(4, 0), Decision(5, 0), Decision(6, 0),         
    };

    PuyoColor c1 = kumiPuyos[nth].axis;
    PuyoColor c2 = kumiPuyos[nth].child;
    int num_decisions = (c1 == c2) ? 11 : 22;
    
    for (int i = 0; i < num_decisions; i++) {
        const Decision& decision = decisions[i];
        if (!Ctrl::isReachable(field, decision))
            continue;

        Field nextField(field);
        int dropFrames = nextField.framesToDropNext(decision);
        nextField.dropKumiPuyo(decision, kumiPuyos[nth]);

        BasicRensaResult rensaInfo = nextField.simulate();
        if (nextField.color(3, 12) != EMPTY)
            continue;
        
        // Add a new plan.
        Plan plan = previousPlan ? 
            Plan(*previousPlan, decision, nextField, rensaInfo.score + previousPlan->totalScore(),
                 rensaInfo.chains, previousPlan->totalFrames(), dropFrames + rensaInfo.frames + previousPlan->totalFrames(), rensaInfo.chains > 0) :                 
            Plan(decision, nextField, rensaInfo.score, rensaInfo.chains, 0, dropFrames + rensaInfo.frames, rensaInfo.chains > 0);            
        // If we have fired our rensa, we don't think any more in this time.
        // TODO: We have to think sooner or later.
        if (restDepth > 1 && rensaInfo.chains == 0)
            findAvailablePlansInternal(nextField, &plan, restDepth - 1, nth + 1, kumiPuyos, plans);
        else
            plans.push_back(plan);
    }
}

void findAvailablePlans(const Field& field, int depth, const vector<KumiPuyo>& kumiPuyos, vector<Plan>& plans)
{
    DCHECK(1 <= depth && depth <= 3);

    plans.clear();
    plans.reserve(22 + 22*22 + 22*22*22);
    findAvailablePlansInternal(field, NULL, depth, 0, kumiPuyos, plans);
}

