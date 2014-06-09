#ifndef CORE_BASIC_FIELD_H_
#define CORE_BASIC_FIELD_H_

#include "core/puyo.h"

class BasicField {
 public:
  static const int WIDTH = 6;
  static const int HEIGHT = 12;
  static const int MAP_WIDTH = 1 + WIDTH + 1;
  static const int MAP_HEIGHT = 1 + HEIGHT + 3;

  PuyoColor Get(int x, int y) const {
    return static_cast<PuyoColor>(field_[x][y]);
  }
  void Set(int x, int y, PuyoColor c) {
    field_[x][y] = static_cast<Puyo>(c);
  }

 protected:
  Puyo field_[MAP_WIDTH][MAP_HEIGHT];
};

#endif
