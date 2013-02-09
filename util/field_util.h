#ifndef FIELD_UTIL_H_
#define FIELD_UTIL_H_

#include <glog/logging.h>

// Requirements:
// - Field::Set() and Field::Get() are defined.
// - EMPTY must be zero.

template <class Field>
void DropFlyingPuyos(Field* f) {
  static const int EMPTY = 0;
  for (int x = 1; x <= 6; x++) {
    for (int y = 2; y <= 13; y++) {
      char c = f->Get(x, y);
      if (c != EMPTY && f->Get(x, y - 1) == EMPTY) {
        int ny = y - 1;
        while (true) {
          if (f->Get(x, ny - 1) != EMPTY)
            break;
          ny--;
          CHECK_GT(ny, 0);
        }
        f->Set(x, ny, c);
        f->Set(x, y, EMPTY);
      }
    }
  }
}

#endif  // FIELD_UTIL_H_
