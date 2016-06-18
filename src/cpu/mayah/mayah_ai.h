#ifndef CPU_MAYAH_MAYAH_AI_H_
#define CPU_MAYAH_MAYAH_AI_H_

#include <memory>
#include <string>
#include <vector>

#include "core/plan/plan.h"

#include "mayah_base_ai.h"
#include "evaluator.h"
#include "gazer.h"

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

class MayahAI : public MayahBaseAI {
public:
    static const int DEFAULT_DEPTH = 2;
    static const int DEFAULT_NUM_ITERATION = 3;
    static const int FAST_DEPTH = 2;
    static const int FAST_NUM_ITERATION = 2;

    MayahAI(int argc, char* argv[], std::unique_ptr<Executor> executor = std::unique_ptr<Executor>());
    ~MayahAI() override;

    DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const override;

    void gaze(int frameId, const CoreField& enemyField, const KumipuyoSeq&) override;

    void onGameWillBegin(const FrameRequest&) override;

    // Use this directly in test. Otherwise, use via think.
    // When |specifiedDecisionsOnly| is specified, only that decision will be considered.
    ThoughtResult thinkPlan(int frameId, const CoreField&, const KumipuyoSeq&,
                            const PlayerState& me, const PlayerState& enemy,
                            int depth, int maxIteration, bool fast = false,
                            std::vector<Decision>* specifiedDecisions = nullptr) const;

protected:
    PreEvalResult preEval(const CoreField& currentField) const;
    MidEvalResult midEval(const RefPlan&, const CoreField& currentField,
                          const KumipuyoSeq& restSeq,
                          int currentFrameId, int maxIteration,
                          const PlayerState& me, const PlayerState& enemy,
                          const PreEvalResult&, const GazeResult&) const;
    EvalResult eval(const RefPlan&, const KumipuyoSeq& restSeq, int currentFrameId, int maxIteration,
                    const PlayerState& me, const PlayerState& enemy,
                    const PreEvalResult&, const MidEvalResult&, bool fast, const GazeResult&) const;
    CollectedFeatureCoefScore evalWithCollectingFeature(
        const RefPlan&, const KumipuyoSeq& restSeq, int currentFrameId, int maxIteration,
        const PlayerState& me, const PlayerState& enemy,
        const PreEvalResult&, const MidEvalResult&, bool fast, const GazeResult&) const;

    std::string makeMessageFrom(int frameId, const KumipuyoSeq&, int maxIteration,
                                const PlayerState& me, const PlayerState& enemy,
                                const PreEvalResult&, const MidEvalResult&, const GazeResult&,
                                const Plan& plan, double rensaScore, double virutalRensaScore,
                                bool saturated, bool fast,
                                double thoughtTimeInSeconds) const;

    bool usesDecisionBook_ = true;
    bool usesRensaHandTree_ = true;

    Gazer gazer_;
};

class DebuggableMayahAI : public MayahAI {
public:
    DebuggableMayahAI() : MayahAI(0, nullptr) {}
    DebuggableMayahAI(int argc, char* argv[], std::unique_ptr<Executor> executor = std::unique_ptr<Executor>()) :
        MayahAI(argc, argv, std::move(executor)) {}
    virtual ~DebuggableMayahAI() {}

    using MayahAI::preEval;
    using MayahAI::saveEvaluationParameter;
    using MayahAI::loadEvaluationParameter;
    using MayahAI::makeMessageFrom;

    using MayahAI::gameWillBegin;
    using MayahAI::gameHasEnded;
    using MayahAI::next2AppearedForEnemy;
    using MayahAI::decisionRequestedForEnemy;
    using MayahAI::groundedForEnemy;

    using MayahAI::myPlayerState;
    using MayahAI::enemyPlayerState;
    using MayahAI::evalWithCollectingFeature;

    using MayahAI::mutableMyPlayerState;
    using MayahAI::mutableEnemyPlayerState;

    const Gazer& gazer() const { return gazer_; }

    void setUsesRensaHandTree(bool flag) { usesRensaHandTree_ = flag; }

    void removeNontokopuyoParameter() { evaluationParameterMap_.removeNontokopuyoParameter(); }

    const EvaluationParameterMap& evaluationParameterMap() const { return evaluationParameterMap_; }
    void setEvaluationParameterMap(const EvaluationParameterMap&);
};

#endif // CPU_MAYAH_MAYAH_AI_H_
