// A few template functions for manipulation of Field like classes.
//
// Requirements:
// - Field::Set() and Field::Get() are defined.
// - EMPTY must be zero.

#ifndef UTIL_FIELD_UTIL_H_
#define UTIL_FIELD_UTIL_H_

#include <string>
#include <glog/logging.h>

using namespace std;

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

template <class Field>
void GetRensimQueryString(const Field& f, string* out) {
  bool started = false;
  for (int y = 13; y >= 1; y--) {
    for (int x = 1; x <= 6; x++) {
      char c = f.Get(x, y);
      if (c) {
        started = true;
      }
      if (started) {
        out->push_back(c + '0');
      }
    }
  }
}

#endif  // UTIL_FIELD_UTIL_H_
