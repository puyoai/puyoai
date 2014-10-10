#ifndef CLIENT_CPU_MAYAH_MAYAH_AI_H_
#define CLIENT_CPU_MAYAH_MAYAH_AI_H_

#include <memory>
#include <string>
#include <vector>

#include "core/client/ai/ai.h"
#include "core/algorithm/plan.h"

#include "book_field.h"
#include "evaluator.h"
#include "feature_parameter.h"
#include "gazer.h"

class CoreField;
class DropDecision;
class KumipuyoSeq;

struct ThoughtResult {
    ThoughtResult() {}
    ThoughtResult(const Plan& plan, bool isRensaPlan, double rensaScore, double virtualRensaScore) :
       plan(plan), isRensaPlan(isRensaPlan), rensaScore(rensaScore), virtualRensaScore(virtualRensaScore) {}

    Plan plan;
    bool isRensaPlan;
    double rensaScore;
    double virtualRensaScore;
};

class MayahAI : public AI {
public:
    static const int DEFAULT_DEPTH = 2;
    static const int DEFAULT_NUM_ITERATION = 3;
    static const int FAST_NUM_ITERATION = 1;

    MayahAI(int argc, char* argv[]);
    virtual ~MayahAI();

    virtual DropDecision think(int frameId, const PlainField&, const KumipuyoSeq&,
                               const AdditionalThoughtInfo&) override;
    virtual DropDecision thinkFast(int frameId, const PlainField&, const KumipuyoSeq&,
                                   const AdditionalThoughtInfo&) override;

    virtual void onGameWillBegin(const FrameRequest&) override;
    virtual void onGameHasEnded(const FrameRequest&) override;
    virtual void onEnemyDecisionRequested(const FrameRequest&) override;
    virtual void onEnemyNext2Appeared(const FrameRequest&) override;

    // Use this directly in test. Otherwise, use via think/thinkFast.
    ThoughtResult thinkPlan(int frameId, const CoreField&, const KumipuyoSeq&, const AdditionalThoughtInfo&,
                            int depth, int maxIteration);

protected:
    EvalResult eval(const RefPlan&, const CoreField& currentField, int currentFrameId, int maxIteration) const;
    CollectedFeature evalWithCollectingFeature(const RefPlan&, const CoreField& currentField, int currentFrameId, int maxIteration) const;

    std::string makeMessageFrom(int frameId, const CoreField&, const KumipuyoSeq&, int maxIteration,
                                const ThoughtResult&, double thoughtTimeInSeconds) const;

    // For debugging purpose.
    void reloadParameter();

    std::unique_ptr<FeatureParameter> featureParameter_;
    std::vector<BookField> books_;

    CoreField enemyField_;
    int enemyDecisonRequestFrameId_;

    Gazer gazer_;
    int thoughtMaxRensa_ = 0;
    int thoughtMaxScore_ = 0;
};

class DebuggableMayahAI : public MayahAI {
public:
    DebuggableMayahAI(int argc, char* argv[]) : MayahAI(argc, argv) {}
    virtual ~DebuggableMayahAI() {}

    using MayahAI::additionalThoughtInfo;
    using MayahAI::think;
    using MayahAI::reloadParameter;
    using MayahAI::makeMessageFrom;

    using MayahAI::gameWillBegin;
    using MayahAI::gameHasEnded;
    using MayahAI::enemyNext2Appeared;
    using MayahAI::enemyDecisionRequested;
    using MayahAI::enemyGrounded;
};

#endif
