#ifndef CORE_CTRL_H_
#define CORE_CTRL_H_

#include <glog/logging.h>
#include <string>
#include <stdio.h>
#include <vector>

#include "core/key.h"
#include "core/key_set.h"
#include "core/puyo_color.h"

class Decision;
class KumipuyoPos;
class PlainField;

class Ctrl {
public:
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
    static bool isQuickturn(const PlainField&, const KumipuyoPos&);
    static bool isReachableFastpath(const PlainField&, const Decision&);
    static void moveHorizontally(int x, KeySetSeq* ret);
    static void add(Key b, KeySetSeq* ret);
};

#endif  // CORE_CTRL_H_
