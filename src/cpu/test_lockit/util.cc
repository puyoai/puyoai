#include "cpu/test_lockit/util.h"

#include "core/core_field.h"
#include "core/puyo_color.h"

#include "color.h"

TLColor toTLColor(PuyoColor pc)
{
    switch (pc) {
    case PuyoColor::EMPTY:  return TLColor::TL_EMPTY;
    case PuyoColor::OJAMA:  return TLColor::TL_OJAMA;
    case PuyoColor::RED:    return TLColor::TL_RED;
    case PuyoColor::BLUE:   return TLColor::TL_BLUE;
    case PuyoColor::YELLOW: return TLColor::TL_YELLOW;
    case PuyoColor::GREEN:  return TLColor::TL_GREEN;
    default: CHECK(false);
    }
}

PuyoColor toPuyoColor(TLColor c)
{
    switch (c) {
    case TLColor::TL_EMPTY:  return PuyoColor::EMPTY;
    case TLColor::TL_OJAMA:  return PuyoColor::OJAMA;
    case TLColor::TL_RED:    return PuyoColor::RED;
    case TLColor::TL_BLUE:   return PuyoColor::BLUE;
    case TLColor::TL_YELLOW: return PuyoColor::YELLOW;
    case TLColor::TL_GREEN:  return PuyoColor::GREEN;
    default: CHECK(false);
    }
}

CoreField toCoreField(int f[6][18]) {
  CoreField cf;
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 14; ++j) {
      if (f[i][j] == TL_EMPTY)
	break;
      cf.dropPuyoOn(i + 1, toPuyoColor(f[i][j]));
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
