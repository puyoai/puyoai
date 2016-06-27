#ifndef CPU_MAYAH_PATTERN_THINKER_H_
#define CPU_MAYAH_PATTERN_THINKER_H_

#include "base/executor.h"
#include "base/time.h"
#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq.h"
#include "core/pattern/decision_book.h"
#include "core/pattern/pattern_book.h"
#include "core/plan/plan.h"

#include "evaluator.h"
#include "gazer.h"

struct ThoughtResult {
    ThoughtResult() {}
    ThoughtResult(const Plan& plan, double rensaScore, double virtualRensaScore,
                  const MidEvalResult& midEvalResult, const std::string& message) :
        plan(plan), rensaScore(rensaScore), virtualRensaScore(virtualRensaScore),
        midEvalResult(midEvalResult), message(message)
    {
    }

    Plan plan;
    double rensaScore;
    double virtualRensaScore;
    MidEvalResult midEvalResult;
    std::string message;
};

class PatternThinker {
public:
    static const int DEFAULT_DEPTH = 2;
    static const int DEFAULT_NUM_ITERATION = 3;
    static const int FAST_DEPTH = 2;
    static const int FAST_NUM_ITERATION = 2;

    PatternThinker(const EvaluationParameterMap& evaluationParameterMap,
                   const DecisionBook& decisionBook,
                   const PatternBook& patternBook,
                   Executor* executor);

    DropDecision think(int frameId, const CoreField& f, const KumipuyoSeq& kumipuyoSeq,
                       const PlayerState& me, const PlayerState& enemy,
                       const GazeResult& gazeResult, bool fast,
                       bool usesDecisionBook, bool usesRensaHandTree) const;

    // Use this directly in test. Otherwise, use via think.
    // When |specifiedDecisionsOnly| is specified, only that decision will be considered.
    ThoughtResult thinkPlan(int frameId, const CoreField&, const KumipuyoSeq&,
                            const PlayerState& me, const PlayerState& enemy,
                            int depth, int maxIteration, const GazeResult&, bool fast = false,
                            bool usesDecisionBook = true, bool usesRensaHandTree = true,
                            std::vector<Decision>* specifiedDecisions = nullptr) const;

    CollectedFeatureCoefScore evalWithCollectingFeature(
        const RefPlan&, const KumipuyoSeq& restSeq, int currentFrameId, int maxIteration,
        const PlayerState& me, const PlayerState& enemy,
        const MidEvalResult&, bool fast, bool usesRensaHandTree, const GazeResult& gazeResult) const;

private:
    MidEvalResult midEval(const RefPlan&, const CoreField& currentField,
                          const KumipuyoSeq& restSeq,
                          int currentFrameId, int maxIteration,
                          const PlayerState& me, const PlayerState& enemy,
                          const GazeResult&, bool usesRensaHandTree) const;
    EvalResult eval(const RefPlan&, const KumipuyoSeq& restSeq, int currentFrameId, int maxIteration,
                    const PlayerState& me, const PlayerState& enemy,
                    const MidEvalResult&, bool fast, bool usesRensaHandTree, const GazeResult&) const;

    std::string makeMessageFrom(int frameId, const KumipuyoSeq&, int maxIteration,
                                const PlayerState& me, const PlayerState& enemy,
                                const MidEvalResult&, const GazeResult&,
                                const Plan& plan, double rensaScore, double virutalRensaScore,
                                bool saturated, bool fast, bool usesRensaHandTree) const;

    const EvaluationParameterMap& evaluationParameterMap_;
    const DecisionBook& decisionBook_;
    const PatternBook& patternBook_;
    Executor* executor_;
};

#endif // CPU_MAYAH_PATTERN_THINKER_H_
