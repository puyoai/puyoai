#include "color.h"

namespace test_lockit {

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

} // namespace test_lockit
