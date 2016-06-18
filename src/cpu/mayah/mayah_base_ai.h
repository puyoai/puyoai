#ifndef CPU_MAYAH_BASE_AI_H_
#define CPU_MAYAH_BASE_AI_H_

#include <memory>
#include <set>
#include <vector>

#include "base/executor.h"
#include "core/client/ai/ai.h"
#include "core/pattern/decision_book.h"
#include "core/pattern/pattern_book.h"

#include "evaluation_parameter.h"

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

class MayahBaseAI : public AI {
public:
    MayahBaseAI(int argc, char* argv[], const char* name, std::unique_ptr<Executor> executor);

protected:
    bool loadEvaluationParameter();
    bool saveEvaluationParameter() const;

    DropDecision thinkByBeamSearch(int frameId, const CoreField&, const KumipuyoSeq&,
                                   const PlayerState& me, const PlayerState& enemy, bool fast) const;
    SearchResult run(const std::vector<State>& initialStates, KumipuyoSeq, int maxSearchTurns) const;
    std::pair<double, int> evalSuperLight(const CoreField& fieldBeforeRensa) const;

    EvaluationParameterMap evaluationParameterMap_;
    DecisionBook decisionBook_;
    PatternBook patternBook_;
    std::unique_ptr<Executor> executor_;

private:
};

#endif // CPU_MAYAH_BASE_AI_H_
