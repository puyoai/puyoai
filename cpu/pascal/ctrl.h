#ifndef __CTRL_H__
#define __CTRL_H__

#include "core/field.h"
#include <string>
#include <stdio.h>
#include <vector>

/*
enum Key {
  KEY_NONE = 0, KEY_UP = 1, KEY_RIGHT = 2, KEY_DOWN = 3, KEY_LEFT = 4, KEY_RIGHT_TURN = 5, KEY_LEFT_TURN = 6
};
*/

class KeyTuple {
 public:
  Key b1, b2;
  KeyTuple(Key bb1, Key bb2) : b1(bb1), b2(bb2) {
  }
  KeyTuple() : b1(KEY_NONE), b2(KEY_NONE) {}
};

class KumipuyoPos {
 public:
  int y, x;
  int r;
  KumipuyoPos() : y(0), x(0), r(0) {}
  KumipuyoPos(int x0, int y0, int r0) : y(y0), x(x0), r(r0) {}
  KumipuyoPos(const Decision &d);
  std::string debugString() const;
  static const KumipuyoPos INIT;
};

class Ctrl {
 public:
  Ctrl();
  ~Ctrl();

  static bool isReachable(const Field& field, const Decision& decision);
  static bool isReachableOnline(const Field& field, const KumipuyoPos &goal,
                                KumipuyoPos start);
  static bool isQuickturn(const Field& field, const KumipuyoPos& k);
  static bool getControl(const Field& field, const Decision& decision,
                         std::vector<KeyTuple>* ret);
  static bool getControlOnline(const Field& field, KumipuyoPos goal, KumipuyoPos start, std::vector<KeyTuple>* ret);
  static std::string buttonsDebugString(const std::vector<KeyTuple>& ret);

 private:
  static void moveHorizontally(int x, std::vector<KeyTuple>* ret);
  static void add(Key b, std::vector<KeyTuple>* ret);
  static void add2(Key b1, Key b2, std::vector<KeyTuple>* ret);
};

#endif  // __CTRL_H__
