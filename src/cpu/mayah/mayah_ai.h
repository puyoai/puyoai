#ifndef CLIENT_CPU_MAYAH_MAYAH_AI_H_
#define CLIENT_CPU_MAYAH_MAYAH_AI_H_

#include <memory>
#include <string>
#include <vector>

#include "core/client/ai/ai.h"

#include "feature_parameter.h"
#include "gazer.h"

class CoreField;
class DropDecision;
class Evaluator;
class KumipuyoSeq;
class Plan;
class RefPlan;

class MayahAI : public AI {
public:
    MayahAI();
    ~MayahAI();

    virtual void gameWillBegin(const FrameData&) OVERRIDE;
    virtual void gameHasEnded(const FrameData&) OVERRIDE;
    virtual DropDecision think(int frameId, const PlainField&, const KumipuyoSeq&) OVERRIDE;
    virtual DropDecision thinkFast(int frameId, const PlainField&, const KumipuyoSeq&) OVERRIDE;
    virtual void enemyGrounded(const FrameData&) OVERRIDE;
    virtual void enemyNext2Appeared(const FrameData&) OVERRIDE;

    // Use this directly in test. Otherwise, use via think/thinkFast.
    Plan thinkPlan(int frameId, const CoreField&, const KumipuyoSeq&, bool fast) const;

    void initializeGazerForTest(int frameId) { gazer_.initializeWith(frameId); }

protected:
    std::string makeMessageFrom(int frameId, const CoreField&, const KumipuyoSeq&, bool fast,
                                const Plan&, double thoughtTimeInSeconds) const;

    std::unique_ptr<FeatureParameter> featureParameter_;
    Gazer gazer_;
};

#endif
