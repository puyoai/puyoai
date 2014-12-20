#ifndef CLIENT_CPU_MAYAH_MAYAH_AI_H_
#define CLIENT_CPU_MAYAH_MAYAH_AI_H_

#include <memory>
#include <string>
#include <vector>

#include "base/executor.h"
#include "core/client/ai/ai.h"
#include "core/algorithm/plan.h"

#include "opening_book.h"
#include "evaluation_parameter.h"
#include "evaluator.h"
#include "gazer.h"
#include "decision_book.h"

class CoreField;
class DropDecision;
class KumipuyoSeq;

struct ThoughtResult {
    ThoughtResult() {}
    ThoughtResult(const Plan& plan, bool isRensaPlan, double rensaScore, double virtualRensaScore,
                  const MidEvalResult& midEvalResult, const std::string& message) :
        plan(plan), isRensaPlan(isRensaPlan), rensaScore(rensaScore), virtualRensaScore(virtualRensaScore),
        midEvalResult(midEvalResult), message(message)
    {
    }

    Plan plan;
    bool isRensaPlan;
    double rensaScore;
    double virtualRensaScore;
    MidEvalResult midEvalResult;
    std::string message;
};

class MayahAI : public AI {
public:
    static const int DEFAULT_DEPTH = 2;
    static const int DEFAULT_NUM_ITERATION = 3;
    static const int FAST_NUM_ITERATION = 1;

    MayahAI(int argc, char* argv[], Executor* executor = nullptr);
    virtual ~MayahAI();

    virtual DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                               const AdditionalThoughtInfo&, bool fast) override;

    virtual void gaze(int frameId, const CoreField& enemyField, const KumipuyoSeq&) override;

    virtual void onGameWillBegin(const FrameRequest&) override;

    // Use this directly in test. Otherwise, use via think.
    // When |specifiedDecisionsOnly| is specified, only that decision will be considered.
    ThoughtResult thinkPlan(int frameId, const CoreField&, const KumipuyoSeq&,
                            const PlayerState& me, const PlayerState& enemy,
                            int depth, int maxIteration,
                            std::vector<Decision>* specifiedDecisions = nullptr);

protected:
    PreEvalResult preEval(const CoreField& currentField);
    EvalResult eval(const RefPlan&, const CoreField& currentField, int currentFrameId, int maxIteration,
                    const PlayerState& me, const PlayerState& enemy,
                    const PreEvalResult&, const MidEvalResult&, const GazeResult&) const;
    CollectedFeature evalWithCollectingFeature(
        const RefPlan&, const CoreField& currentField, int currentFrameId, int maxIteration,
        const PlayerState& me, const PlayerState& enemy,
        const PreEvalResult&, const MidEvalResult&, const GazeResult&) const;

    std::string makeMessageFrom(int frameId, const CoreField&, const KumipuyoSeq&, int maxIteration,
                                const PlayerState& me, const PlayerState& enemy,
                                const PreEvalResult&, const MidEvalResult&, const GazeResult&,
                                const Plan& plan, double rensaScore, double virutalRensaScore, double thoughtTimeInSeconds) const;

    // For debugging purpose.
    void reloadParameter();

    std::unique_ptr<EvaluationParameter> evaluationParameter_;
    std::vector<OpeningBookField> books_;
    DecisionBook decisionBook_;
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
    using MayahAI::reloadParameter;
    using MayahAI::makeMessageFrom;

    using MayahAI::gameWillBegin;
    using MayahAI::gameHasEnded;
    using MayahAI::enemyNext2Appeared;
    using MayahAI::enemyDecisionRequested;
    using MayahAI::enemyGrounded;

    using MayahAI::myPlayerState;
    using MayahAI::enemyPlayerState;
    using MayahAI::evalWithCollectingFeature;

    const Gazer& gazer() const { return gazer_; }

    const EvaluationParameter& evaluationParameter() const { return *evaluationParameter_; }
    void setEvaluationParameter(const EvaluationParameter& parameter) { *evaluationParameter_ = parameter; }
};

#endif
