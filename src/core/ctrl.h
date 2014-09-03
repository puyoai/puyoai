#ifndef CORE_CTRL_H_
#define CORE_CTRL_H_

#include <glog/logging.h>
#include <string>
#include <stdio.h>
#include <vector>

#include "core/constant.h"
#include "core/key.h"
#include "core/key_set.h"
#include "core/kumipuyo.h"
#include "core/puyo_color.h"

class Decision;
class KumipuyoPos;
class PlainField;

struct MovingKumipuyoState {
    explicit MovingKumipuyoState(const KumipuyoPos& pos) : pos(pos) {}

    KumipuyoPos pos;
    int restFramesTurnProhibited = 0;
    int restFramesToAcceptQuickTurn = 0;
    int restFramesForFreefall = FRAMES_FREE_FALL;
    bool grounded = false;
};

class Ctrl {
public:
    static void moveKumipuyo(const PlainField&, const KeySet&, MovingKumipuyoState*, bool* downAccepted = nullptr);

    static KeySetSeq findKeyStrokeByDijkstra(const PlainField&, const MovingKumipuyoState&, const Decision&);

    static bool isReachable(const PlainField&, const Decision&);
    /**
     * Judges if kumi-puyo can be moved to |goal| from |start|.
     * goal.y is ignored. Always tries to place puyo on top of existing puyos.
     * (because it is not needed for normal game, not nazopuyo)
     */
    static bool isReachableOnline(const PlainField&, const KumipuyoPos& goal, const KumipuyoPos& start);

    static bool getControl(const PlainField&, const Decision&, KeySetSeq* ret);

    static bool getControlOnline(const PlainField&, const KumipuyoPos& goal, const KumipuyoPos& start, KeySetSeq* ret);

private:
    // Move kumipuyo using only arrow key. True is returned only when DOWN is accepted.
    static void moveKumipuyoByArrowKey(const PlainField&, const KeySet&, MovingKumipuyoState*, bool* downAccepted);
    static void moveKumipuyoByTurnKey(const PlainField&, const KeySet&, MovingKumipuyoState*);
    static void moveKumipuyoByFreefall(const PlainField&, MovingKumipuyoState*);

    static bool isQuickturn(const PlainField&, const KumipuyoPos&);
    static bool isReachableFastpath(const PlainField&, const Decision&);
    static void moveHorizontally(int x, KeySetSeq* ret);
    static void add(Key b, KeySetSeq* ret);
};

#endif  // CORE_CTRL_H_
