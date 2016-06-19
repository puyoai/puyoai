#ifndef CPU_MAYAH_MAYAH_AI_H_
#define CPU_MAYAH_MAYAH_AI_H_

#include <memory>
#include <string>
#include <vector>

#include "mayah_base_ai.h"
#include "evaluator.h"
#include "gazer.h"

class CoreField;
class DropDecision;
class KumipuyoSeq;

class MayahAI : public MayahBaseAI {
public:
    MayahAI(int argc, char* argv[], std::unique_ptr<Executor> executor = std::unique_ptr<Executor>());
    ~MayahAI() override;

    DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const override;
    ThoughtResult thinkPlan(int frameId, const CoreField&, const KumipuyoSeq&,
                            const PlayerState& me, const PlayerState& enemy,
                            int depth, int maxIteration, bool fast = false,
                            std::vector<Decision>* specifiedDecisions = nullptr) const;
    CollectedFeatureCoefScore evalWithCollectingFeature(
        const RefPlan&, const KumipuyoSeq& restSeq, int currentFrameId, int maxIteration,
        const PlayerState& me, const PlayerState& enemy,
        const MidEvalResult&, bool fast, const GazeResult& gazeResult) const;

protected:
    bool usesDecisionBook_ = true;
    bool usesRensaHandTree_ = true;
};

class DebuggableMayahAI : public MayahAI {
public:
    DebuggableMayahAI() : MayahAI(0, nullptr) {}
    DebuggableMayahAI(int argc, char* argv[], std::unique_ptr<Executor> executor = std::unique_ptr<Executor>()) :
        MayahAI(argc, argv, std::move(executor)) {}
    virtual ~DebuggableMayahAI() {}

    using MayahAI::saveEvaluationParameter;
    using MayahAI::loadEvaluationParameter;

    using MayahAI::gameWillBegin;
    using MayahAI::gameHasEnded;
    using MayahAI::next2AppearedForEnemy;
    using MayahAI::decisionRequestedForEnemy;
    using MayahAI::groundedForEnemy;

    using MayahAI::myPlayerState;
    using MayahAI::enemyPlayerState;

    using MayahAI::mutableMyPlayerState;
    using MayahAI::mutableEnemyPlayerState;

    void setUsesRensaHandTree(bool flag) { usesRensaHandTree_ = flag; }

    void removeNontokopuyoParameter() { evaluationParameterMap_.removeNontokopuyoParameter(); }

    const EvaluationParameterMap& evaluationParameterMap() const { return evaluationParameterMap_; }
    void setEvaluationParameterMap(const EvaluationParameterMap&);
};

#endif // CPU_MAYAH_MAYAH_AI_H_
