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
#include "pattern_thinker.h"
#include "rush_thinker.h"
#include "side_thinker.h"

class MayahBaseAI : public AI {
public:
    MayahBaseAI(int argc, char* argv[], const char* name, std::unique_ptr<Executor> executor);

    const Gazer& gazer() const { return gazer_; }

    void onGameWillBegin(const FrameRequest&) override;
    void gaze(int frameId, const CoreField& enemyField, const KumipuyoSeq&) override;

protected:
    bool loadEvaluationParameter();
    bool saveEvaluationParameter() const;

    DropDecision thinkByBeamSearch(int frameId, const CoreField&, const KumipuyoSeq&,
                                   const PlayerState& me, const PlayerState& enemy, bool fast) const;

    EvaluationParameterMap evaluationParameterMap_;
    DecisionBook decisionBook_;
    PatternBook patternBook_;
    std::unique_ptr<Executor> executor_;

    std::unique_ptr<BeamThinker> beam_thinker_;
    std::unique_ptr<PatternThinker> pattern_thinker_;
    std::unique_ptr<RushThinker> rush_thinker_;
    std::unique_ptr<SideThinker> side_thinker_;

    Gazer gazer_;
};

#endif // CPU_MAYAH_BASE_AI_H_
