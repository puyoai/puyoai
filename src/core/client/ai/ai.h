#ifndef CORE_CLIENT_AI_H_
#define CORE_CLIENT_AI_H_

#include <string>

#include "core/client/ai/drop_decision.h"
#include "core/client/connector/client_connector.h"
#include "core/field/core_field.h"

struct FrameRequest;
class KumipuyoSeq;
class PlainField;

// TODO(mayah): This struct will contain zenkeshi info etc.
struct AdditionalThoughtInfo {
};

// AI is a utility class of AI.
// All you have to do is to implement think().
class AI {
public:
    const std::string& name() const { return name_; }

    void runLoop();

protected:
    explicit AI(int argc, char* argv[], const std::string& name);

    // gameWillBegin will be called just before a new game will begin.
    // FrameRequest will contain NEXT and NEXT2 puyos.
    // Please initialize your AI in this function.
    virtual void gameWillBegin(const FrameRequest&) {}

    // When a game has finished, gameHasEnd will be called.
    virtual void gameHasEnded(const FrameRequest&) {}

    // think will be called when ai should decide the next hand.
    // Basically, this will be called when NEXT2 has appeared.
    // You will have around 300 ms to decide your hand.
    virtual DropDecision think(int frameId, const PlainField&, const KumipuyoSeq&,
                               const AdditionalThoughtInfo&) = 0;

    // thinkFast will be called when ai should decide the next hand immediately.
    // Basically this will be called when ojamas are dropped.
    // You will have around 30 ms to decide your hand.
    virtual DropDecision thinkFast(int frameId, const PlainField& field, const KumipuyoSeq& next,
                                   const AdditionalThoughtInfo& info)
    {
        return think(frameId, field, next, info);
    }

    // When enemy's puyo is grounded, this callback will be called.
    // You can detect enemy has started his rensa in this callback.
    virtual void enemyGrounded(const FrameRequest&) {}

    // When enemy's NEXT2 has appeared, this callback will be called.
    // You can update the enemy information here.
    virtual void enemyNext2Appeared(const FrameRequest&) {}

    // Should reconsider just before sending next decision.
    void requestReconsider() { reconsiderRequested_ = true; }

    void setDifensive(bool flag) { difensive_ = flag; }

private:
    void resetCurrentField(const CoreField&);

    std::string name_;
    ClientConnector connector_;
    CoreField field_;  // estimated my field.
    int hand_;
    bool reconsiderRequested_;
    bool difensive_;
};

#endif
