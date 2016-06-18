#ifndef CPU_MAYAH_BASE_AI_H_
#define CPU_MAYAH_BASE_AI_H_

#include <memory>
#include <set>
#include <vector>

#include "base/executor.h"
#include "core/client/ai/ai.h"
#include "core/pattern/decision_book.h"
#include "core/pattern/pattern_book.h"

#include "beam_thinker.h"
#include "evaluation_parameter.h"

class MayahBaseAI : public AI {
public:
    MayahBaseAI(int argc, char* argv[], const char* name, std::unique_ptr<Executor> executor);

protected:
    bool loadEvaluationParameter();
    bool saveEvaluationParameter() const;

    DropDecision thinkByBeamSearch(int frameId, const CoreField&, const KumipuyoSeq&,
                                   const PlayerState& me, const PlayerState& enemy, bool fast) const;

    EvaluationParameterMap evaluationParameterMap_;
    DecisionBook decisionBook_;
    PatternBook patternBook_;
    std::unique_ptr<Executor> executor_;

private:
    BeamThinker beam_thinker_;
};

#endif // CPU_MAYAH_BASE_AI_H_
