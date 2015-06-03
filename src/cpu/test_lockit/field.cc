#include "field.h"

#include "color.h"

namespace test_lockit {

bool IsTLFieldEmpty(const int field[6][kHeight]) {
  for (int i = 0; i < 6; ++i) {
    if (field[i][0] != TLColor::TL_EMPTY) {
      return false;
    }
  }
  return true;
}

bool IsColorPuyo(const int c) {
  switch(c) {
  case TLColor::TL_RED:
  case TLColor::TL_BLUE:
  case TLColor::TL_YELLOW:
  case TLColor::TL_GREEN:
    return true;
  }
  return false;
}

}  // namespace test_lockit
