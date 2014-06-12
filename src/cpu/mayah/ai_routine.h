#ifndef CLIENT_CPU_AI_ROUTINE_H_
#define CLIENT_CPU_AI_ROUTINE_H_

#include <string>
#include <vector>

#include "core/client/ai/ai.h"
#include "enemy_info.h"
#include "evaluation_feature.h"

class CoreField;
class DropDecision;
class KumipuyoSeq;
class Plan;

struct EvalResult {
    EvalResult(double evaluation, const std::string& message);

    double evaluationScore;
    std::string message;
};

class AIRoutine : public AI {
public:
    static const int NUM_KEY_PUYOS = 1;

    AIRoutine();

protected:
    virtual void gameWillBegin(const FrameData&) OVERRIDE;
    virtual void gameHasEnd(const FrameData&) OVERRIDE;
    virtual DropDecision think(int frameId, const PlainField&, const Kumipuyo& next1, const Kumipuyo& next2) OVERRIDE;
    virtual void enemyGrounded(const FrameData&) OVERRIDE;
    virtual void enemyNext2Appeared(const FrameData&) OVERRIDE;

    EvalResult eval(int currentFrameId, const Plan&) const;
    DropDecision decide(int frameId, const CoreField&, const KumipuyoSeq&, const EnemyInfo&);

    EvaluationParams evaluationParams_;
    EnemyInfo enemyInfo_;
};

#endif
