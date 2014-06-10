#ifndef CORE_CTRL_H_
#define CORE_CTRL_H_

#include <glog/logging.h>
#include <string>
#include <stdio.h>
#include <vector>

#include "core/key.h"
#include "core/puyo_color.h"

class Decision;
class KumipuyoPos;
class PlainField;

class KeyTuple {
public:
    KeyTuple(Key bb1, Key bb2) : b1(bb1), b2(bb2) {}
    KeyTuple() : b1(Key::KEY_NONE), b2(Key::KEY_NONE) {}

    bool hasSameKey(KeyTuple k) const
    {
        return b1 == k.b1 || b1 == k.b2 || b2 == k.b1 || b2 == k.b2;
    }

public:
    Key b1, b2;
};

class Ctrl {
public:
    static bool isReachable(const PlainField&, const Decision&);
    /**
     * Judges if kumi-puyo can be moved to goal from start.
     * goal.y is ignored. Always tries to place puyo on top of existing puyos.
     * (because it is not needed for normal game, not nazopuyo)
     */
    static bool isReachableOnline(const PlainField&, const KumipuyoPos &goal, const KumipuyoPos& start);

    static bool isQuickturn(const PlainField&, const KumipuyoPos&);

    static bool getControl(const PlainField&, const Decision&, std::vector<KeyTuple>* ret);

    static bool getControlOnline(const PlainField&, const KumipuyoPos& goal, const KumipuyoPos& start, std::vector<KeyTuple>* ret);

    static std::string buttonsDebugString(const std::vector<KeyTuple>& ret);

private:
    static void moveHorizontally(int x, std::vector<KeyTuple>* ret);
    static void add(Key b, std::vector<KeyTuple>* ret);
    static void add2(Key b1, Key b2, std::vector<KeyTuple>* ret);
};

#endif  // CORE_CTRL_H_
