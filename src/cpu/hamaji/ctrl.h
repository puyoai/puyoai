#ifndef CTRL_H_
#define CTRL_H_

#include <string>
#include <stdio.h>
#include <vector>

#include "core/kumipuyo.h"
#include "field.h"

enum Key {
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT_TURN,
    KEY_LEFT_TURN,
    KEY_START,
    KEY_NONE,
};

class KeyTuple {
 public:
  KeyTuple(Key bb1, Key bb2) : b1(bb1), b2(bb2) {}
  KeyTuple() : b1(Key::KEY_NONE), b2(Key::KEY_NONE) {}

  bool hasSameKey(KeyTuple k) const {
    return b1 == k.b1 || b1 == k.b2 || b2 == k.b1 || b2 == k.b2;
  }

 public:
  Key b1, b2;
};

class LC {
 public:
  LC();
  ~LC();

  static bool isReachable(const LF& field, const Decision& decision);
  static bool isReachableOnline(const LF& field, const KumipuyoPos &goal,
                                KumipuyoPos start);
  static bool isQuickturn(const LF& field, const KumipuyoPos& k);
  static bool getControl(const LF& field, const Decision& decision,
                         std::vector<KeyTuple>* ret);
  static bool getControlOnline(const LF& field, KumipuyoPos goal, KumipuyoPos start, std::vector<KeyTuple>* ret);
  static std::string buttonsDebugString(const std::vector<KeyTuple>& ret);

 private:
  static void moveHorizontally(int x, std::vector<KeyTuple>* ret);
  static void add(Key b, std::vector<KeyTuple>* ret);
  static void add2(Key b1, Key b2, std::vector<KeyTuple>* ret);
};

#endif  // CTRL_H_
