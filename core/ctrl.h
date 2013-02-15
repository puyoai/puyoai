#ifndef CORE_CTRL_H_
#define CORE_CTRL_H_

#include <glog/logging.h>
#include <string>
#include <stdio.h>
#include <vector>

#include "core/decision.h"
#include "core/key.h"

class KeyTuple {
 public:
  Key b1, b2;
  KeyTuple(Key bb1, Key bb2) : b1(bb1), b2(bb2) {
  }
  KeyTuple() : b1(KEY_NONE), b2(KEY_NONE) {}

  bool hasSameKey(KeyTuple k) const {
    return b1 == k.b1 || b1 == k.b2 || b2 == k.b1 || b2 == k.b2;
  }
};

class KumipuyoPos {
 public:
  static const KumipuyoPos& InitialPos();

  KumipuyoPos() : y(0), x(0), r(0) {}
  KumipuyoPos(int x0, int y0, int r0) : y(y0), x(x0), r(r0) {}
  KumipuyoPos(const Decision& d);
  std::string debugString() const;

 public:
  int y, x;
  int r;
};

inline const KumipuyoPos& KumipuyoPos::InitialPos() {
  static const KumipuyoPos initial_pos(3, 12, 0);
  return initial_pos;
}

inline KumipuyoPos::KumipuyoPos(const Decision& d) {
  if (d.r == 3) {
    this->x = d.x + 1;
  } else {
    this->x = d.x;
  }
  this->r = d.r;
  this->y = 2;
}

inline std::string KumipuyoPos::debugString() const {
  char buf[256];
  snprintf(buf, 256, "<y=%d,x=%d,r=%d>", y, x, r);
  return std::string(buf);
}

class Ctrl {
 public:
  template<typename Field>
  static bool isReachable(const Field& field, const Decision& decision);
  template<typename Field>
  static bool isReachableOnline(const Field& field, const KumipuyoPos &goal,
                                KumipuyoPos start);
  template<typename Field>
  static bool isQuickturn(const Field& field, const KumipuyoPos& k);
  template<typename Field>
  static bool getControl(const Field& field, const Decision& decision,
                         std::vector<KeyTuple>* ret);
  template<typename Field>
  static bool getControlOnline(const Field& field, KumipuyoPos goal, KumipuyoPos start, std::vector<KeyTuple>* ret);

  static std::string buttonsDebugString(const std::vector<KeyTuple>& ret);

 private:
  static void moveHorizontally(int x, std::vector<KeyTuple>* ret);
  static void add(Key b, std::vector<KeyTuple>* ret);
  static void add2(Key b1, Key b2, std::vector<KeyTuple>* ret);
};

template<typename Field>
bool Ctrl::getControl(const Field& field, const Decision& decision,
                             std::vector<KeyTuple>* ret) {
  ret->clear();

  if (!isReachable(field, decision)) {
    return false;
  }
  int x = decision.x;
  int r = decision.r;

  switch (r) {
    case 0:
      moveHorizontally(x - 3, ret);
      break;

    case 1:
      add(KEY_RIGHT_TURN, ret);
      moveHorizontally(x - 3, ret);
      break;

    case 2:
      moveHorizontally(x - 3, ret);
      if (x < 3) {
        add(KEY_RIGHT_TURN, ret);
        add(KEY_RIGHT_TURN, ret);
      } else if (x > 3) {
        add(KEY_LEFT_TURN, ret);
        add(KEY_LEFT_TURN, ret);
      } else {
        if (field.Get(4, 12)) {
          if (field.Get(2, 12)) {
            // fever's quick turn
            add(KEY_RIGHT_TURN, ret);
            add(KEY_RIGHT_TURN, ret);
          } else {
            add(KEY_LEFT_TURN, ret);
            add(KEY_LEFT_TURN, ret);
          }
        } else {
          add(KEY_RIGHT_TURN, ret);
          add(KEY_RIGHT_TURN, ret);
        }
      }
      break;

    case 3:
      add(KEY_LEFT_TURN, ret);
      moveHorizontally(x - 3, ret);
      break;

    default:
      LOG(FATAL) << r;
  }

  add(KEY_DOWN, ret);

  return true;
}

inline void Ctrl::add(Key b, std::vector<KeyTuple>* ret) {
  ret->push_back(KeyTuple(b, KEY_NONE));
}

inline void Ctrl::add2(Key b1, Key b2, std::vector<KeyTuple>* ret) {
  ret->push_back(KeyTuple(b1, b2));
}

inline void Ctrl::moveHorizontally(int x, std::vector<KeyTuple>* ret) {
  if (x < 0) {
    for (int i = 0; i < -x; i++) {
      add(KEY_LEFT, ret);
    }
  } else if (x > 0) {
    for (int i = 0; i < x; i++) {
      add(KEY_RIGHT, ret);
    }
  }
}

template<typename Field>
bool Ctrl::isReachable(const Field& field, const Decision& decision) {
  return isReachableOnline(field, KumipuyoPos(decision.x, 1, decision.r),
                           KumipuyoPos::InitialPos());
}

template<typename Field>
bool Ctrl::isQuickturn(const Field& field, const KumipuyoPos& k) {
  // assume that k.r == 0 or 2
  return (field.Get(k.x - 1, k.y) && field.Get(k.x + 1, k.y));
}

/**
 * Judges if kumi-puyo can be moved to goal from start.
 * goal.y is ignored. Always tries to place puyo on top of existing puyos.
 * (because it is not needed for normal game, not nazopuyo)
 */
template<typename Field>
bool Ctrl::isReachableOnline(const Field& field, const KumipuyoPos& goal, KumipuyoPos start) {
  std::vector<KeyTuple> ret;
  return getControlOnline(field, goal, start, &ret);
}

// returns null if not reachable
template<typename Field>
bool Ctrl::getControlOnline(const Field& field, KumipuyoPos goal, KumipuyoPos start, std::vector<KeyTuple>* ret) {
  ret->clear();
  while(1) {
    if (goal.x == start.x && goal.r == start.r) {
      break;
    }

    // for simpicity, direct child-puyo upwards
    // TODO(yamaguchi): eliminate unnecessary moves
    if (start.r == 1) {
      add(KEY_LEFT_TURN, ret);
      start.r = 0;
    } else if (start.r == 3) {
      add(KEY_RIGHT_TURN, ret);
      start.r = 0;
    } else if (start.r == 2) {
      if (isQuickturn(field, start)) {
        // do quick turn
        add(KEY_RIGHT_TURN, ret);
        add(KEY_RIGHT_TURN, ret);
        start.y++;
      } else {
        if (field.Get(start.x - 1, start.y)) {
          add(KEY_LEFT_TURN, ret);
          add(KEY_LEFT_TURN, ret);
        } else {
          add(KEY_RIGHT_TURN, ret);
          add(KEY_RIGHT_TURN, ret);
        }
      }
      start.r = 0;
    }
    if (goal.x == start.x) {
      switch(goal.r) {
        case 0:
          break;
        case 1:
          if (field.Get(start.x + 1, start.y)) {
            if (field.Get(start.x + 1, start.y + 1) ||
                !field.Get(start.x, start.y - 1)) {
              return false;
            }
            // turn inversely to avoid kicking wall
            add(KEY_LEFT_TURN, ret);
            add(KEY_LEFT_TURN, ret);
            add(KEY_LEFT_TURN, ret);
          } else {
            add(KEY_RIGHT_TURN, ret);
          }
          break;
        case 3:
          if (field.Get(start.x - 1, start.y)) {
            if (field.Get(start.x - 1, start.y + 1) ||
                !field.Get(start.x, start.y - 1)) {
              return false;
            }
            add(KEY_RIGHT_TURN, ret);
            add(KEY_RIGHT_TURN, ret);
            add(KEY_RIGHT_TURN, ret);
          } else {
            add(KEY_LEFT_TURN, ret);
          }
          break;
        case 2:
          if (field.Get(start.x - 1, start.y)) {
            add(KEY_RIGHT_TURN, ret);
            add(KEY_RIGHT_TURN, ret);
          } else {
            add(KEY_LEFT_TURN, ret);
            add(KEY_LEFT_TURN, ret);
          }
          break;
      }
      break;
    }

    // direction to move horizontally
    if (goal.x > start.x) {
      // move to right
      if (!field.Get(start.x + 1, start.y)) {
        add(KEY_RIGHT, ret);
        start.x++;
      } else {  // hits a wall
        // climb if possible
        /*
          aBb
          .A@
          .@@.
        */
        // pivot puyo cannot go up anymore
        if (start.y >= 13) return false;
        // check "b"
        if (field.Get(start.x + 1, start.y + 1)) {
          return false;
        }
        if (field.Get(start.x, start.y - 1) || isQuickturn(field, start)) {
          // can climb by kicking the ground or quick turn. In either case,
          // kumi-puyo is never moved because right side is blocked

          add(KEY_LEFT_TURN, ret);
          add(KEY_LEFT_TURN, ret);
          start.y++;
          if (!field.Get(start.x - 1, start.y + 1)) {
            add(KEY_RIGHT_TURN, ret);
            add(KEY_RIGHT, ret);
          } else {
            // if "a" in the figure is filled, kicks wall. we can omit right key.
            add(KEY_RIGHT_TURN, ret);
          }
          add(KEY_RIGHT_TURN, ret);
          start.x++;
        } else {
          return false;
        }
      }
    } else {
      // move to left
      if (!field.Get(start.x - 1, start.y)) {
        add(KEY_LEFT, ret);
        start.x--;
      } else {  // hits a wall
        // climb if possible
        /*
          bBa
          @A.
          @@@.
        */
        // pivot puyo cannot go up anymore
        if (start.y >= 13) return false;
        // check "b"
        if (field.Get(start.x - 1, start.y + 1)) {
          return false;
        }
        if (field.Get(start.x, start.y - 1) || isQuickturn(field, start)) {
          // can climb by kicking the ground or quick turn. In either case,
          // kumi-puyo is never moved because left side is blocked
          add(KEY_RIGHT_TURN, ret);
          add(KEY_RIGHT_TURN, ret);
          start.y++;
          if (!field.Get(start.x + 1, start.y)) {
            add(KEY_LEFT_TURN, ret);
            add(KEY_LEFT, ret);
          } else {
            // if "a" in the figure is filled, kicks wall. we can omit left key.
            add(KEY_LEFT_TURN, ret);
          }
          add(KEY_LEFT_TURN, ret);
          start.x--;
        } else {
          return false;
        }
      }
    }
  }
  add(KEY_DOWN, ret);
  //LOG(INFO) << buttonsDebugString();
  return true;
}

inline std::string Ctrl::buttonsDebugString(const std::vector<KeyTuple>& ret) {
  char cmds[] = " ^>v<AB";
  // caution: this string is used by test cases.
  std::string out;
  for (int i = 0; i < int(ret.size()); i++) {
    if (i != 0) {
      out += ',';
    }
    if (ret[i].b1) {
      out += cmds[ret[i].b1];
    }
    if (ret[i].b2) {
      out += cmds[ret[i].b2];
    }
  }
  return out;
}

#endif  // CORE_CTRL_H_
