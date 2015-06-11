#ifndef CORE_KUMIPUYO_MOVING_STATE_H_
#define CORE_KUMIPUYO_MOVING_STATE_H_

#include "core/frame.h"
#include "core/kumipuyo_pos.h"

class PlainField;
class KeySet;

class KumipuyoMovingState {
public:
    constexpr KumipuyoMovingState() : pos(KumipuyoPos()) {}
    constexpr explicit KumipuyoMovingState(const KumipuyoPos& pos) : pos(pos) {}

    static KumipuyoMovingState initialState() { return KumipuyoMovingState(KumipuyoPos::initialPos()); }

    bool isInitialPosition() const { return pos.x == 3 && pos.y == 12 && pos.r == 0; }

    friend bool operator==(const KumipuyoMovingState& lhs, const KumipuyoMovingState& rhs);
    friend bool operator!=(const KumipuyoMovingState& lhs, const KumipuyoMovingState& rhs) { return !(lhs == rhs); }
    friend bool operator<(const KumipuyoMovingState& lhs, const KumipuyoMovingState& rhs);
    friend bool operator>(const KumipuyoMovingState& lhs, const KumipuyoMovingState& rhs) { return rhs < lhs; }

    void moveKumipuyo(const PlainField&, const KeySet&, bool* downAccepted);

public:
    // TODO(mayah): Make this private.

    // Move kumipuyo using only arrow key. |downAccepted| gets true when DOWN is accepted.
    void moveKumipuyoByArrowKey(const PlainField&, const KeySet&, bool* downAccepted);
    void moveKumipuyoByTurnKey(const PlainField&, const KeySet&);
    void moveKumipuyoByFreefall(const PlainField&);

    KumipuyoPos pos;
    int restFramesTurnProhibited = 0;
    int restFramesArrowProhibited = 0;
    int restFramesToAcceptQuickTurn = 0;
    int restFramesForFreefall = FRAMES_FREE_FALL;
    int numGrounded = 0;
    bool grounding = false;
    bool grounded = false;
};

#endif
