#ifndef CORE_PUYO_CONTROLLER_H_
#define CORE_PUYO_CONTROLLER_H_

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

// TODO(mayah): Currently this file is a bit messy. Refactoring is desired.

struct MovingKumipuyoState {
    MovingKumipuyoState() {}
    explicit MovingKumipuyoState(const KumipuyoPos& pos) : pos(pos) {}

    friend bool operator==(const MovingKumipuyoState& lhs, const MovingKumipuyoState& rhs)
    {
        return std::tie(lhs.pos,
                        lhs.restFramesTurnProhibited,
                        lhs.restFramesArrowProhibited,
                        lhs.restFramesToAcceptQuickTurn,
                        lhs.restFramesForFreefall,
                        lhs.grounded) ==
            std::tie(rhs.pos,
                     rhs.restFramesTurnProhibited,
                     rhs.restFramesArrowProhibited,
                     rhs.restFramesToAcceptQuickTurn,
                     rhs.restFramesForFreefall,
                     rhs.grounded);
    }
    friend bool operator!=(const MovingKumipuyoState& lhs, const MovingKumipuyoState& rhs) { return !(lhs == rhs); }
    friend bool operator<(const MovingKumipuyoState& lhs, const MovingKumipuyoState& rhs)
    {
        return std::tie(lhs.pos,
                        lhs.restFramesTurnProhibited,
                        lhs.restFramesArrowProhibited,
                        lhs.restFramesToAcceptQuickTurn,
                        lhs.restFramesForFreefall,
                        lhs.grounded) <
            std::tie(rhs.pos,
                     rhs.restFramesTurnProhibited,
                     rhs.restFramesArrowProhibited,
                     rhs.restFramesToAcceptQuickTurn,
                     rhs.restFramesForFreefall,
                     rhs.grounded);
    }
    friend bool operator>(const MovingKumipuyoState& lhs, const MovingKumipuyoState& rhs) { return rhs < lhs; }

    KumipuyoPos pos;
    int restFramesTurnProhibited = 0;
    int restFramesArrowProhibited = 0;
    int restFramesToAcceptQuickTurn = 0;
    int restFramesForFreefall = FRAMES_FREE_FALL;
    bool grounded = false;
};

class PuyoController {
public:
    struct ParameterForDuel {
        static const int FRAMES_CONTINUOUS_TURN_PROHIBITED = 1;
        static const int FRAMES_CONTINUOUS_ARROW_PROHIBITED = 0;
    };

    struct ParameterForWii {
        static const int FRAMES_CONTINUOUS_TURN_PROHIBITED = 1;
        static const int FRAMES_CONTINUOUS_ARROW_PROHIBITED = 1;
    };

    template<typename Parameter = ParameterForDuel>
    static void moveKumipuyo(const PlainField&, const KeySet&, MovingKumipuyoState*, bool* downAccepted = nullptr);

    static bool isReachable(const PlainField&, const Decision&);
    static bool isReachableFrom(const PlainField&, const MovingKumipuyoState&, const Decision&);

    // Finds a key stroke to move puyo from |MovingKumipuyoState| to |Decision|.
    // When there is not such a way, the returned KeySetSeq would be empty sequence.
    static KeySetSeq findKeyStroke(const CoreField&, const MovingKumipuyoState&, const Decision&);
    // TODO(mayah): Should we put this here?
    static KeySetSeq findKeyStrokeForWii(const CoreField&, const Decision&);

    // Fast, but usable in limited situation.
    static KeySetSeq findKeyStrokeFastpath(const CoreField&, const MovingKumipuyoState&, const Decision&);
    static KeySetSeq findKeyStrokeFastpathForWii(const CoreField&, const MovingKumipuyoState&, const Decision&);
    // This is faster, but might output worse key stroke.
    static KeySetSeq findKeyStrokeOnline(const PlainField&, const MovingKumipuyoState&, const Decision&);

    // This is slow, but precise.
    template<typename Parameter = ParameterForDuel>
    static KeySetSeq findKeyStrokeByDijkstra(const PlainField&, const MovingKumipuyoState&, const Decision&);

private:
    // Move kumipuyo using only arrow key. |downAccepted| gets true when DOWN is accepted.
    template<typename Parameter>
    static void moveKumipuyoByArrowKey(const PlainField&, const KeySet&, MovingKumipuyoState*, bool* downAccepted);
    template<typename Parameter>
    static void moveKumipuyoByTurnKey(const PlainField&, const KeySet&, MovingKumipuyoState*, bool* needsFreefallProcess);
    static void moveKumipuyoByFreefall(const PlainField&, MovingKumipuyoState*);

    static bool isReachableFastpath(const PlainField&, const Decision&);

    static KeySetSeq findKeyStrokeOnlineInternal(const PlainField&, const MovingKumipuyoState&, const Decision&);
};

#endif  // CORE_PUYO_CONTROLLER_H_
