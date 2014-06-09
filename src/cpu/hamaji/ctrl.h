#ifndef CTRL_H_
#define CTRL_H_

#include <string>
#include <stdio.h>
#include <vector>

#include "../../core/ctrl.h"

#include "field.h"

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
