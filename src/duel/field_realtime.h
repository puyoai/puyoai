#ifndef DUEL_FIELD_REALTIME_H_
#define DUEL_FIELD_REALTIME_H_

#include <vector>

#include "core/decision.h"
#include "core/key_set_seq.h"
#include "core/kumipuyo_moving_state.h"
#include "core/kumipuyo_seq.h"
#include "core/next_puyo.h"
#include "core/plain_field.h"
#include "core/user_event.h"

// TODO(mayah): We need to do refactoring this. This class is really messy.
class FrameContext;

class FieldRealtime {
public:
    enum class SimulationState {
        STATE_LEVEL_SELECT,   // initial state
        STATE_PREPARING_NEXT,
        STATE_PLAYABLE,       // A user is moving puyo
        STATE_DROPPING,       // on dropping
        STATE_GROUNDING,
        STATE_VANISH,         // on vanishing
        STATE_VANISHING,
        STATE_OJAMA_DROPPING, // on ojama dropping
        STATE_OJAMA_GROUNDING,
        STATE_DEAD,
    };

    FieldRealtime(int playerId, const KumipuyoSeq&);

    int playerId() const { return playerId_; }

    bool isDead() const { return simulationState_ == SimulationState::STATE_DEAD; }
    bool userPlayable() const { return simulationState_ == SimulationState::STATE_PLAYABLE; }

    // Utility functions to be used by duel server.
    UserEvent userEvent() const { return userEvent_; }
    bool playable() const { return playable_; }

    void setKeySetSeq(const KeySetSeq& kss) { keySetSeq_ = kss; }
    KeySet frontKeySet() const { return keySetSeq_.empty() ? KeySet() : keySetSeq_.front(); }
    void dropFrontKeySet()
    {
        if (keySetSeq_.size() > 1 || (keySetSeq_.size() == 1 && keySetSeq_.front() != KeySet(Key::DOWN)))
            keySetSeq_.removeFront();
    }
    const KeySetSeq& keySetSeq() const { return keySetSeq_; }
    const Decision& lastDecision() const { return lastDecision_; }

    bool hasZenkeshi() const { return hasZenkeshi_; }

    int score() const { return score_; }

    int ojama() const { return numFixedOjama_ + numPendingOjama_; }
    int numFixedOjama() const { return numFixedOjama_; }
    int numPendingOjama() const { return numPendingOjama_; }
    void addPendingOjama(int num) { numPendingOjama_ += num; }
    int reduceOjama(int num);
    void commitOjama() { numFixedOjama_ += numPendingOjama_; numPendingOjama_ = 0; }
    std::vector<int> determineColumnOjamaAmount();

    const PlainField& field() const { return field_; }
    const KumipuyoSeq& kumipuyoSeq() const { return kumipuyoSeq_; }
    // If NEXT2 is delaying, this does not contain NEXT2.
    KumipuyoSeq visibleKumipuyoSeq() const;
    Kumipuyo kumipuyo(int nth = 0) const;
    const KumipuyoPos& kumipuyoPos() const { return kms_.pos; }
    const KumipuyoMovingState& kumipuyoMovingState() const { return kms_; }

    // Gives a key input to the field, and control puyo. Returns true if a key
    // input is accepted. FrameContext will collect events when playing frames.
    // Currently, only ojama related events will be collected.
    bool playOneFrame(const KeySet&, FrameContext*);

    // Testing only.
    void skipLevelSelect();
    void skipPreparingNext();
    SimulationState simulationState() const { return simulationState_; }
    bool isSleeping() const { return sleepFor_ > 0; }
    void forceSetField(const PlainField& pf) { field_ = pf; }

private:
    void init();

    // Returns true if we need to drop more.
    bool drop1Frame();

    void transitToStatePreparingNext();

    bool onStateLevelSelect();
    bool onStatePreparingNext();
    bool onStatePlayable(const KeySet&, bool* accepted);
    bool onStateDropping();
    bool onStateGrounding();
    bool onStateVanish(FrameContext*);
    bool onStateVanishing();
    bool onStateOjamaDropping();
    bool onStateOjamaGrounding();
    bool onStateDead();

    int playerId_;

    SimulationState simulationState_ = SimulationState::STATE_LEVEL_SELECT;
    int sleepFor_ = 0;

    PlainField field_; // Since there will be puyos in the air, CoreField cannot be used.
    KumipuyoSeq kumipuyoSeq_;
    UserEvent userEvent_;
    bool playable_;

    KumipuyoMovingState kms_;
    KeySetSeq keySetSeq_;
    Decision lastDecision_;

    int score_ = 0;
    int scoreConsumed_ = 0;
    int numFixedOjama_ = 0;
    int numPendingOjama_ = 0;

    int ojamaDroppingAmount_ = 0;

    bool dropFast_ = false;
    int dropFrameIndex_ = 0;
    int dropRestFrames_ = 0;

    bool allowsQuick_ = false;

    bool hasZenkeshi_ = false;

    bool ojama_dropping_;
    std::vector<int> ojama_position_;
    bool drop_animation_;
    int current_chains_;

    int delayFramesWNextAppear_;
    bool sent_wnext_appeared_;
};

#endif  // DUEL_FIELD_REALTIME_H_
