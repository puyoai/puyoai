#ifndef DUEL_FIELD_REALTIME_H_
#define DUEL_FIELD_REALTIME_H_

#include <vector>

#include "core/field/core_field.h"
#include "core/frame_data.h"
#include "core/key_set.h"
#include "core/kumipuyo.h"
#include "core/next_puyo.h"
#include "core/server/connector/connector_frame_response.h"
#include "core/state.h"

// TODO(mayah): We need to do refactoring this. This class is really messy.
class FrameContext;

class FieldRealtime {
public:
    const double INITIAL_DROP_VELOCITY = 3.0;
    const double MAX_DROP_VELOCITY = 32.0;
    const double DROP_ACCELARATION_PER_FRAME = 1.2;
    const double DROP_1BLOCK_THRESHOLD = 32.0;

    enum class SimulationState {
        STATE_LEVEL_SELECT, // initial state
        STATE_PLAYABLE,     // A user is moving puyo
        STATE_VANISH,       // on vanishing
        STATE_DROP,         // on dropping
        STATE_OJAMA,        // on ojama dropping
    };

    FieldRealtime(int playerId, const KumipuyoSeq&);

    // Gives a key input to the field, and control puyo. Returns true if a key
    // input is accepted. FrameContext will collect events when playing frames.
    // Currently, only ojama related events will be collected.
    bool playOneFrame(const KeySet&, FrameContext*);

    // Checks if a player is dead.
    bool isDead() const { return isDead_; }

    // Utility functions to be used by duel server.
    PlayerFrameData playerFrameData() const;
    UserState userState() const { return userState_; }
    Key getKey(const Decision&) const;

    int score() const { return score_; }
    int ojama() const { return numFixedOjama_ + numPendingOjama_; }
    int numFixedOjama() const { return numFixedOjama_; }
    int numPendingOjama() const { return numPendingOjama_; }

    int playerId() const { return playerId_; }
    bool userPlayable() const { return simulationState_ == SimulationState::STATE_PLAYABLE; }

    void addPendingOjama(int num) { numPendingOjama_ += num; }
    int reduceOjama(int num);
    void commitOjama() { numFixedOjama_ += numPendingOjama_; numPendingOjama_ = 0; }

    std::vector<int> determineColumnOjamaAmount();

    const CoreField& field() const { return field_; }
    const KumipuyoSeq& kumipuyoSeq() const { return kumipuyoSeq_; }
    Kumipuyo kumipuyo(int nth = 0) const;
    const KumipuyoPos& kumipuyoPos() const { return kumipuyoPos_; }
    PuyoColor puyoColor(NextPuyoPosition) const;

    // Testing only.
    void skipLevelSelect();
    SimulationState simulationState() const { return simulationState_; }
    bool isSleeping() const { return sleepFor_ > 0; }
    void forceSetField(const CoreField& cf) { field_ = cf; }

private:
    void init();

    // Returns true if we need to drop more.
    bool drop1Frame();

    bool playInternal(const KeySet&, bool* ground);
    void prepareNextPuyo();
    void finishChain(FrameContext*);
    bool tryVanish(FrameContext*);
    bool tryDrop(FrameContext*);
    bool tryOjama();
    bool tryUserState(const KeySet&);

    int playerId_;

    SimulationState simulationState_ = SimulationState::STATE_LEVEL_SELECT;
    int sleepFor_ = 0;

    CoreField field_;
    KumipuyoSeq kumipuyoSeq_;
    KumipuyoPos kumipuyoPos_;
    UserState userState_;
    bool isDead_ = false;

    int score_ = 0;
    int scoreConsumed_ = 0;
    int numFixedOjama_ = 0;
    int numPendingOjama_ = 0;

    double dropVelocity_ = 0.0;
    double dropAmount_ = 0.0;

    bool ojama_dropping_;
    std::vector<int> ojama_position_;
    bool drop_animation_;
    int frames_for_free_fall_;
    int current_chains_;
    int restFramesToAcceptQuickTurn_;
    bool is_zenkesi_ = false;

    int delayFramesWNextAppear_;
    bool sent_wnext_appeared_;
};

#endif  // DUEL_FIELD_REALTIME_H_
