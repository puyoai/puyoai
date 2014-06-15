#ifndef CLIENT_CPU_AI_ROUTINE_H_
#define CLIENT_CPU_AI_ROUTINE_H_

#include <memory>
#include <string>
#include <vector>

#include "core/client/ai/ai.h"

#include "evaluation_feature.h"
#include "gazer.h"

class CoreField;
class DropDecision;
class Evaluator;
class KumipuyoSeq;
class RefPlan;

class AIRoutine : public AI {
public:
    AIRoutine();
    ~AIRoutine();

protected:
    virtual void gameWillBegin(const FrameData&) OVERRIDE;
    virtual void gameHasEnd(const FrameData&) OVERRIDE;
    virtual DropDecision think(int frameId, const PlainField&, const Kumipuyo& next1, const Kumipuyo& next2) OVERRIDE;
    virtual void enemyGrounded(const FrameData&) OVERRIDE;
    virtual void enemyNext2Appeared(const FrameData&) OVERRIDE;

    std::unique_ptr<Evaluator> evaluator_;
    Gazer gazer_;
};

#endif
