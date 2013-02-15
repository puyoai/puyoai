#ifndef __PLAN_H_
#define __PLAN_H_

#include <string>

#include "drop_decision.h"
#include "field.h"

class Plan {
public:
    Plan(const Decision&, const Field&, int totalScore, int totalChains,
         int initiatingFrames, int totalFrames, bool isRensaPlan);
    Plan(const Plan& previousPlan, const Decision&, const Field&, int totalScore, int totalChains,
         int initiatingFrames, int totalFrames, bool isRensaPlan);

    const Decision& firstHandDecision() const { return m_decisions[0]; }
    const Decision& decision(int nth) const { return m_decisions[nth]; }
    int numDecisions() const { return m_decisions.size(); }

    int totalScore() const { return m_totalScore; }
    int totalChains() const { return m_totalChains; }

    int initiatingFrames() const { return m_initiatingFrames; }
    int totalFrames() const { return m_totalFrames; }

    bool isRensaPlan() const { return m_isRensaPlan; }

    std::string decisionText() const;

    const Field& field() const { return m_field; }
    
private:
    std::vector<Decision> m_decisions;
    Field m_field; // Future field (after the rensa has been finished).
    int m_totalScore; // The score we can get in this plan.
    int m_totalChains; // The number of chains
    int m_initiatingFrames; // The estimated number of frames which is necessary to initiate the last hand of this plan.
    int m_totalFrames; // The estimated number of frames where this plan has been finished.
    bool m_isRensaPlan; // true if this plan will vanish something.
};

inline Plan::Plan(const Decision& decision, const Field& field, int totalScore, int totalChains,
                  int initiatingFrames, int totalFrames, bool isRensaPlan)
    : m_field(field)
    , m_totalScore(totalScore)
    , m_totalChains(totalChains)
    , m_initiatingFrames(initiatingFrames)
    , m_totalFrames(totalFrames)
    , m_isRensaPlan(isRensaPlan)
{
    m_decisions.push_back(decision);
}

inline Plan::Plan(const Plan& plan, const Decision& decision, const Field& field, int totalScore, int totalChains,
                  int initiatingFrames, int totalFrames, bool isRensaPlan)
    : m_decisions(plan.m_decisions)
    , m_field(field)
    , m_totalScore(totalScore)
    , m_totalChains(totalChains)
    , m_initiatingFrames(initiatingFrames)
    , m_totalFrames(totalFrames)
    , m_isRensaPlan(isRensaPlan)
{
    m_decisions.push_back(decision);
}

#endif
