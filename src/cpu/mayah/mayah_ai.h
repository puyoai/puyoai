#ifndef CLIENT_CPU_MAYAH_MAYAH_AI_H_
#define CLIENT_CPU_MAYAH_MAYAH_AI_H_

#include <memory>
#include <string>
#include <vector>

#include "base/executor.h"
#include "core/client/ai/ai.h"
#include "core/algorithm/plan.h"

#include "decision_book.h"
#include "evaluation_parameter.h"
#include "evaluator.h"
#include "gazer.h"
#include "pattern_book.h"

class CoreField;
class DropDecision;
class KumipuyoSeq;

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

class MayahAI : public AI {
public:
    static const int DEFAULT_DEPTH = 2;
    static const int DEFAULT_NUM_ITERATION = 3;
    static const int DEEP_DEPTH = 3;
    static const int DEEP_NUM_ITERATION = 2;
    static const int FAST_DEPTH = 2;
    static const int FAST_NUM_ITERATION = 2;

    MayahAI(int argc, char* argv[], Executor* executor = nullptr);
    virtual ~MayahAI();

    virtual DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                               const PlayerState& me, const EnemyState& enemy, bool fast) const override;

    virtual void gaze(int frameId, const CoreField& enemyField, const KumipuyoSeq&) override;

    virtual void onGameWillBegin(const FrameRequest&) override;

    // Use this directly in test. Otherwise, use via think.
    // When |specifiedDecisionsOnly| is specified, only that decision will be considered.
    ThoughtResult thinkPlan(int frameId, const CoreField&, const KumipuyoSeq&,
                            const PlayerState& me, const EnemyState& enemy,
                            int depth, int maxIteration,
                            std::vector<Decision>* specifiedDecisions = nullptr) const;

protected:
    PreEvalResult preEval(const CoreField& currentField) const;
    MidEvalResult midEval(const RefPlan&, const CoreField& currentField, int currentFrameId, int maxIteration,
                          const PlayerState& me, const EnemyState& enemy,
                          const PreEvalResult&, const GazeResult&) const;
    EvalResult eval(const RefPlan&, int currentFrameId, int maxIteration,
                    const PlayerState& me, const EnemyState& enemy,
                    const PreEvalResult&, const MidEvalResult&, const GazeResult&) const;
    CollectedFeatureCoefScore evalWithCollectingFeature(
        const RefPlan&, int currentFrameId, int maxIteration,
        const PlayerState& me, const EnemyState& enemy,
        const PreEvalResult&, const MidEvalResult&, const GazeResult&) const;

    std::string makeMessageFrom(int frameId, const KumipuyoSeq&, int maxIteration,
                                const PlayerState& me, const EnemyState& enemy,
                                const PreEvalResult&, const MidEvalResult&, const GazeResult&,
                                const Plan& plan, double rensaScore, double virutalRensaScore,
                                bool saturated,
                                double thoughtTimeInSeconds) const;

    bool saveEvaluationParameter() const;
    bool loadEvaluationParameter();

    EvaluationParameterMap evaluationParameterMap_;
    DecisionBook decisionBook_;
    PatternBook patternBook_;

    bool usesDecisionBook_ = true;

    Executor* executor_;

    Gazer gazer_;
};

class DebuggableMayahAI : public MayahAI {
public:
    DebuggableMayahAI() : MayahAI(0, nullptr) {}
    DebuggableMayahAI(int argc, char* argv[], Executor* executor = nullptr) : MayahAI(argc, argv, executor) {}
    virtual ~DebuggableMayahAI() {}

    using MayahAI::preEval;
    using MayahAI::saveEvaluationParameter;
    using MayahAI::loadEvaluationParameter;
    using MayahAI::makeMessageFrom;

    using MayahAI::gameWillBegin;
    using MayahAI::gameHasEnded;
    using MayahAI::enemyNext2Appeared;
    using MayahAI::enemyDecisionRequested;
    using MayahAI::enemyGrounded;

    using MayahAI::myPlayerState;
    using MayahAI::enemyPlayerState;
    using MayahAI::evalWithCollectingFeature;

    using MayahAI::mutableMyPlayerState;
    using MayahAI::mutableEnemyPlayerState;

    const Gazer& gazer() const { return gazer_; }

    void removeNontokopuyoParameter() { evaluationParameterMap_.removeNontokopuyoParameter(); }
    const EvaluationParameterMap& evaluationParameterMap() const { return evaluationParameterMap_; }
    const EvaluationParameter& evaluationParameter(EvaluationMode mode) const
    {
        return evaluationParameterMap().parameter(mode);
    }
    void setEvaluationParameterMap(const EvaluationParameterMap&);
};

#endif
