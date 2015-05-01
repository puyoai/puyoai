#ifndef CORE_CLIENT_AI_H_
#define CORE_CLIENT_AI_H_

#include <string>

#include "core/client/ai/drop_decision.h"
#include "core/client/connector/client_connector.h"
#include "core/kumipuyo_seq.h"
#include "core/player_state.h"

class CoreField;
class PlainField;
struct FrameRequest;

// AI is a utility class of AI.
// You need to implement think() at least.
class AI {
public:
    virtual ~AI();
    const std::string& name() const { return name_; }

    void runLoop();

    // Set AI's behavior. If true, you can rethink next decision when the enemy has started his rensa.
    void setBehaviorRethinkAfterOpponentRensa(bool flag) { behaviorRethinkAfterOpponentRensa_ = flag; }

protected:
    AI(int argc, char* argv[], const std::string& name);
    explicit AI(const std::string& name);

    // think will be called when AI should decide the next decision.
    // Basically, this will be called when NEXT2 has appeared.
    // |frameId| is the frameId that you will get to start moving your puyo.
    // In other words, it's a kind of 'frameInitiated'.
    // |fast| will be true when AI should decide the next decision immeidately,
    // e.g. ojamas are dropped or the enemy has fired some rensa
    // (if you set behavior). This might be also called when field is inconsistent
    // in wii_server.
    // If |fast| is true, you will have 30 ms to decide your hand.
    // Otherwise, you will have at least 300 ms to decide your hand.
    // |KumipuyoSeq| will have at least 2 kumipuyos. When we know more Kumipuyo sequence,
    // it might contain more. It's up to you if you will use >=3 kumipuyos.
    virtual DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                               const PlayerState& me, const PlayerState& enemy, bool fast) const = 0;

    // gaze will be called when AI should gaze the enemy's field.
    // |frameId| is the frameId where the enemy has started moving his puyo.
    // His moving puyo is the front puyo of the KumipuyoSeq.
    // KumipuyoSeq has at least 2 kumipuyos. When we know more Kumipuyo sequence,
    // it might contain more.
    // Since gaze might be called in the same frame as think(), you shouldn't consume
    // much time for gaze.
    virtual void gaze(int frameId, const CoreField& enemyField, const KumipuyoSeq&);

    // ----------------------------------------------------------------------
    // Callbacks. If you'd like to customize your AI, it is good if you could use
    // the following hook methods.
    // These callbacks will be called from the corresponding method.
    // i.e. onX() will be called from X().

    virtual void onGameWillBegin(const FrameRequest&) {}
    virtual void onGameHasEnded(const FrameRequest&) {}

    virtual void onDecisionRequestedForMe(const FrameRequest&) {}
    virtual void onGroundedForMe(const FrameRequest&) {}
    virtual void onOjamaDroppedForMe(const FrameRequest&) {}
    virtual void onNext2AppearedForMe(const FrameRequest&) {}

    virtual void onDecisionRequestedForEnemy(const FrameRequest&) {}
    virtual void onGroundedForEnemy(const FrameRequest&) {}
    virtual void onOjamaDroppedForEnemy(const FrameRequest&) {}
    virtual void onNext2AppearedForEnemy(const FrameRequest&) {}

    // Should rethink just before sending next decision.
    void requestRethink() { rethinkRequested_ = true; }

    // ----------------------------------------------------------------------
    // Usually, you don't need to care about methods below here.

    // |gameWillBegin| will be called just before a new game will begin.
    // FrameRequest might contain NEXT and NEXT2 puyos, but it's not guaranteed.
    // Please initialize your AI in this function.
    void gameWillBegin(const FrameRequest&);

    // |gameHasEnded| will be called just after a game has ended.
    void gameHasEnded(const FrameRequest&);

    void decisionRequestedForMe(const FrameRequest&);
    void decisionRequestedForEnemy(const FrameRequest&);

    void groundedForMe(const FrameRequest&);
    void groundedForEnemy(const FrameRequest&);

    void ojamaDroppedForMe(const FrameRequest&);
    void ojamaDroppedForEnemy(const FrameRequest&);

    void next2AppearedForMe(const FrameRequest&);
    void next2AppearedForEnemy(const FrameRequest&);

    const PlayerState& myPlayerState() const { return me_; }
    const PlayerState& enemyPlayerState() const { return enemy_; }

    PlayerState* mutableMyPlayerState() { return &me_; }
    PlayerState* mutableEnemyPlayerState() { return &enemy_; }

private:
    friend class AITest;
    friend class Endless;
    friend class Solver;

    static bool isFieldInconsistent(const PlainField& ours, const PlainField& provided);
    static CoreField mergeField(const CoreField& ours, const PlainField& provided, bool ojamaDropped);

    KumipuyoSeq rememberedSequence(int indexFrom) const;

    std::string name_;
    ClientConnector connector_;

    bool rethinkRequested_;
    int enemyDecisionRequestFrameId_;

    PlayerState me_;
    PlayerState enemy_;

    bool behaviorRethinkAfterOpponentRensa_;
};

#endif
