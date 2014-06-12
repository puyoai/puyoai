#ifndef DUEL_FIELD_REALTIME_H_
#define DUEL_FIELD_REALTIME_H_

#include <vector>

#include "core/field/core_field.h"
#include "core/frame_data.h"
#include "core/key.h"
#include "core/kumipuyo.h"
#include "core/server/connector/received_data.h"
#include "core/state.h"

// TODO(mayah): We need to do refactoring this. This class is really messy.
class FrameContext;

class FieldRealtime {
public:
    FieldRealtime(int playerId, const KumipuyoSeq&);

    // Gives a key input to the field, and control puyo. Returns true if a key
    // input is accepted.
    bool PlayOneFrame(Key key, FrameContext*);

    // Pretty print of the field.
    void Print() const;
    void Print(const std::string& debug_message) const;

    // Checks if a player is dead.
    bool isDead() const { return isDead_; }

    PuyoColor GetNextPuyo(int n) const;
    void GetCurrentPuyo(int* x1, int* y1, PuyoColor* c1, int* x2, int* y2, PuyoColor* c2, int* r) const;

    // Utility functions to be used by duel server.
    PlayerFrameData playerFrameData() const;
    std::string GetFieldInfo() const;
    std::string GetYokokuInfo() const;
    UserState userState() const { return userState_; }
    Key GetKey(const Decision&) const;

    int score() const { return score_; }
    int ojama() const { return numFixedOjama_ + numPendingOjama_; }
    int numFixedOjama() const { return numFixedOjama_; }
    int numPendingOjama() const { return numPendingOjama_; }

    int playerId() const { return playerId_; }
    bool IsInUserState() const { return simulationState_ == STATE_USER; }

    void addPendingOjama(int num) { numPendingOjama_ += num; }
    int reduceOjama(int num);
    void commitOjama() { numFixedOjama_ += numPendingOjama_; numPendingOjama_ = 0; }

    std::vector<int> determineColumnOjamaAmount();

    const CoreField& field() const { return field_; }
    const KumipuyoSeq& kumipuyoSeq() const { return kumipuyoSeq_; }
    const KumipuyoPos& kumipuyoPos() const { return kumipuyoPos_; }

    enum SimulationState {
        STATE_USER,     // A user is moving puyo
        STATE_CHIGIRI,  // on chigiri
        STATE_VANISH,   // on vanishing
        STATE_DROP,     // on dropping
        STATE_OJAMA,    // on ojama dropping
    };

    // Testing only.
    SimulationState simulationState() const { return simulationState_; }
    bool isSleeping() const { return sleepFor_ > 0; }

private:
    void Init();

    bool Chigiri();
    bool Drop1line();
    bool PlayInternal(Key key, bool* ground);
    void PrepareNextPuyo();
    void FinishChain(FrameContext*);
    bool TryChigiri();
    bool TryVanish(FrameContext*);
    bool TryDrop(FrameContext*);
    bool TryOjama();

    int playerId_;
    SimulationState simulationState_ = STATE_USER;

    CoreField field_;
    KumipuyoSeq kumipuyoSeq_;
    int sleepFor_ = 0;

    bool ojama_dropping_;
    std::vector<int> ojama_position_;
    KumipuyoPos kumipuyoPos_;
    bool drop_animation_;
    int chigiri_x_;
    int chigiri_y_;
    bool isDead_ = false;
    UserState userState_;
    int frames_for_free_fall_;
    int score_ = 0;
    int consumed_score_ = 0;
    int current_chains_;
    int quickturn_;
    bool is_zenkesi_ = false;
    int dropped_rows_;

    int numFixedOjama_ = 0;
    int numPendingOjama_ = 0;

    int delayFramesWNextAppear_;
    bool sent_wnext_appeared_;
};

#endif  // DUEL_FIELD_REALTIME_H_
