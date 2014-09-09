#ifndef CORE_CTRL_H_
#define CORE_CTRL_H_

#include <glog/logging.h>
#include <string>
#include <stdio.h>
#include <tuple>
#include <vector>

#include "core/constant.h"
#include "core/key.h"
#include "core/key_set.h"
#include "core/kumipuyo.h"
#include "core/puyo_color.h"

class CoreField;
class Decision;
class KumipuyoPos;
class PlainField;

struct MovingKumipuyoState {
    MovingKumipuyoState() {}
    explicit MovingKumipuyoState(const KumipuyoPos& pos) : pos(pos) {}

    friend bool operator==(const MovingKumipuyoState& lhs, const MovingKumipuyoState& rhs)
    {
        return std::tie(lhs.pos,
                        lhs.restFramesTurnProhibited,
                        lhs.restFramesToAcceptQuickTurn,
                        lhs.restFramesForFreefall,
                        lhs.grounded) ==
            std::tie(rhs.pos,
                     rhs.restFramesTurnProhibited,
                     rhs.restFramesToAcceptQuickTurn,
                     rhs.restFramesForFreefall,
                     rhs.grounded);
    }
    friend bool operator!=(const MovingKumipuyoState& lhs, const MovingKumipuyoState& rhs) { return !(lhs == rhs); }
    friend bool operator<(const MovingKumipuyoState& lhs, const MovingKumipuyoState& rhs)
    {
        return std::tie(lhs.pos,
                        lhs.restFramesTurnProhibited,
                        lhs.restFramesToAcceptQuickTurn,
                        lhs.restFramesForFreefall,
                        lhs.grounded) <
            std::tie(rhs.pos,
                     rhs.restFramesTurnProhibited,
                     rhs.restFramesToAcceptQuickTurn,
                     rhs.restFramesForFreefall,
                     rhs.grounded);
    }
    friend bool operator>(const MovingKumipuyoState& lhs, const MovingKumipuyoState& rhs) { return rhs < lhs; }

    KumipuyoPos pos;
    int restFramesTurnProhibited = 0;
    int restFramesToAcceptQuickTurn = 0;
    int restFramesForFreefall = FRAMES_FREE_FALL;
    bool grounded = false;
};

class PuyoController {
public:
    static void moveKumipuyo(const PlainField&, const KeySet&, MovingKumipuyoState*, bool* downAccepted = nullptr);

    static bool isReachable(const PlainField&, const Decision&);
    static bool isReachableFrom(const PlainField&, const MovingKumipuyoState&, const Decision&);

    // Finds a key stroke to move puyo from |MovingKumipuyoState| to |Decision|.
    // When there is not such a way, the returned KeySetSeq would be empty sequence.
    static KeySetSeq findKeyStroke(const CoreField&, const MovingKumipuyoState&, const Decision&);

    static KeySetSeq findKeyStrokeFastpath(const CoreField&, const MovingKumipuyoState&, const Decision&);
    // This is faster, but might output worse key stroke.
    static KeySetSeq findKeyStrokeOnline(const PlainField&, const MovingKumipuyoState&, const Decision&);
    // This is slow, but precise.
    static KeySetSeq findKeyStrokeByDijkstra(const PlainField&, const MovingKumipuyoState&, const Decision&);

private:
    // Move kumipuyo using only arrow key. |downAccepted| gets true when DOWN is accepted.
    static void moveKumipuyoByArrowKey(const PlainField&, const KeySet&, MovingKumipuyoState*, bool* downAccepted);
    static void moveKumipuyoByTurnKey(const PlainField&, const KeySet&, MovingKumipuyoState*, bool* needsFreefallProcess);
    static void moveKumipuyoByFreefall(const PlainField&, MovingKumipuyoState*);

    static bool isReachableFastpath(const PlainField&, const Decision&);

    static KeySetSeq findKeyStrokeOnlineInternal(const PlainField&, const MovingKumipuyoState&, const Decision&);
};

#endif  // CORE_CTRL_H_
