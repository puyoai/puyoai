#include "cpu/test_lockit/util.h"

#include "core/core_field.h"
#include "core/puyo_color.h"

#include "color.h"

namespace test_lockit {

CoreField toCoreField(int f[6][18]) {
  CoreField cf;
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 14; ++j) {
      if (f[i][j] == TL_EMPTY)
	break;
      cf.dropPuyoOn(i + 1, toPuyoColor(TLColor(f[i][j])));
    }
  }
  return cf;
}

void toTLField(const CoreField& cf, int f[6][18]) {
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 14; ++j) {
      f[i][j] = toTLColor(cf.color(i + 1, j + 1));
    }
    for (int j = 14; j < 18; ++j) {
      f[i][j] = TL_EMPTY;
    }
  }
}

}  // namespace test_lockit
