#pragma once

#include "evaluator.h"

#include "core/client/ai/ai.h"

class CoreField;
class RefPlan;

namespace munetoshi {

class AI: public ::AI {
public:
    AI(int argc, char* argv[]);
    virtual ~AI() = default;

    DropDecision think(int frame_id,
            const CoreField& field,
            const KumipuyoSeq& seq,
            const PlayerState& me,
            const PlayerState& enemy,
            bool fast) const override;

protected:
    enum Strategy {
        FIRE, GROW,
    };

    virtual void onGameWillBegin(const FrameRequest&) override;

    virtual DropDecision think_internal(int frame_id, const CoreField& field,
            const KumipuyoSeq& seq) const;

    virtual void onEnemyGrounded(const FrameRequest&) override;

    virtual int evaluate(const CoreField& field, const RefPlan* plan) const;

    Strategy strategy;

private:
    AI(const AI&) = delete;
    AI(AI&&) = delete;
    AI& operator =(const AI&) = delete;
    AI& operator =(AI&&) = delete;
};

}  // namespace munetoshi
