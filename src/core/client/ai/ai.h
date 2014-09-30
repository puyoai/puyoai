#ifndef CORE_CLIENT_AI_H_
#define CORE_CLIENT_AI_H_

#include <string>

#include "core/client/ai/drop_decision.h"
#include "core/client/connector/client_connector.h"
#include "core/field/core_field.h"
#include "core/field/rensa_result.h"

struct FrameRequest;
class KumipuyoSeq;
class PlainField;

// TODO(mayah): This struct will contain zenkeshi info etc.
class AdditionalThoughtInfo {
public:
    bool isRensaOngoing() const { return isRensaOngoing_; }
    const RensaResult& ongoingRensaResult() const { return ongoingRensaResult_; }
    int ongoingRensaFinishingFrameId() const { return finishingRensaFrameId_; }

    void setOngoingRensa(const RensaResult&, int finishingFrameId);
    void unsetOngoingRensa();

private:
    bool isRensaOngoing_ = false;
    int finishingRensaFrameId_ = 0;
    RensaResult ongoingRensaResult_;
};

// AI is a utility class of AI.
// All you have to do is to implement think().
class AI {
public:
    const std::string& name() const { return name_; }

    void runLoop();

protected:
    explicit AI(int argc, char* argv[], const std::string& name);

    // |gameWillBegin| will be called just before a new game will begin.
    // FrameRequest might contain NEXT and NEXT2 puyos, but it's not guaranteed.
    // Please initialize your AI in this function.
    virtual void gameWillBegin(const FrameRequest&) {}

    // |gameHasEnded| will be called just after a game has ended.
    virtual void gameHasEnded(const FrameRequest&) {}

    // |think| will be called when AI should decide the next decision.
    // Basically, this will be called when NEXT2 has appeared.
    // You will have around 300 ms to decide your hand.
    virtual DropDecision think(int frameId, const PlainField&, const KumipuyoSeq&,
                               const AdditionalThoughtInfo&) = 0;

    // |thinkFast| will be called when AI should decide the next decision immediately.
    // Basically this will be called when ojamas are dropped or the enemy has fired some rensa.
    // You will have around 30 ms to decide your hand.
    virtual DropDecision thinkFast(int frameId, const PlainField& field, const KumipuyoSeq& next,
                                   const AdditionalThoughtInfo& info)
    {
        return think(frameId, field, next, info);
    }

    // When enemy will start to move puyo, this callback will be called.
    virtual void enemyDecisionRequest(const FrameRequest&);

    // When enemy's puyo is grounded, this callback will be called.
    // Enemy's rensa is automatically checked, so you don't need to do that. (Use AdditionalThoughtInfo)
    virtual void enemyGrounded(const FrameRequest&);

    // When enemy's NEXT2 has appeared, this callback will be called.
    // You can update the enemy information here.
    virtual void enemyNext2Appeared(const FrameRequest&);

    // Should rethink just before sending next decision.
    void requestRethink() { rethinkRequested_ = true; }

    void setBehaviorDefensive(bool flag) { behaviorDefensive_ = flag; }
    void setBehaviorRethinkAfterOpponentRensa(bool flag) { behaviorRethinkAfterOpponentRensa_ = flag; }

private:
    friend class Solver;

    void resetCurrentField(const CoreField&);

    std::string name_;
    ClientConnector connector_;
    CoreField field_;  // estimated my field.
    AdditionalThoughtInfo additionalThoughtInfo_;
    int hand_;
    bool rethinkRequested_;

    bool behaviorDefensive_;
    bool behaviorRethinkAfterOpponentRensa_;
};

#endif
