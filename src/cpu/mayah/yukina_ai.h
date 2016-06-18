#ifndef CPU_MAYAH_YUKINA_AI_H_
#define CPU_MAYAH_YUKINA_AI_H_

#include <memory>
#include <set>
#include <vector>

#include "mayah_base_ai.h"

struct SearchResult {
    std::set<Decision> firstDecisions;
    int maxChains = 0;
};

struct State {
    State(const CoreField& field, const Decision& firstDecision, double stateScore, int maxChains) :
        field(field), firstDecision(firstDecision), stateScore(stateScore), maxChains(maxChains) {}

    friend bool operator<(const State& lhs, const State& rhs) { return lhs.stateScore < rhs.stateScore; }
    friend bool operator>(const State& lhs, const State& rhs) { return lhs.stateScore > rhs.stateScore; }

    CoreField field;
    Decision firstDecision;
    double stateScore = 0;
    int maxChains = 0;
};

class YukinaAI : public MayahBaseAI {
public:
    YukinaAI(int argc, char* argv[]);

    DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const override;

    SearchResult run(const std::vector<State>& initialStates, KumipuyoSeq, int maxSearchTurns) const;

private:
    std::pair<double, int> evalSuperLight(const CoreField& fieldBeforeRensa) const;

    mutable std::mutex mu_; // for cout
};

#endif // CPU_MAYAH_YUKINA_AI_H_
