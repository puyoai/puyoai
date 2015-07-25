#include "color.h"

namespace test_lockit {

TLColor toTLColor(PuyoColor pc)
{
    switch (pc) {
    case PuyoColor::EMPTY:  return TLColor::EMPTY;
    case PuyoColor::OJAMA:  return TLColor::OJAMA;
    case PuyoColor::RED:    return TLColor::RED;
    case PuyoColor::BLUE:   return TLColor::BLUE;
    case PuyoColor::YELLOW: return TLColor::YELLOW;
    case PuyoColor::GREEN:  return TLColor::GREEN;
    default: CHECK(false);
    }
}

PuyoColor toPuyoColor(TLColor c)
{
    switch (c) {
    case TLColor::EMPTY:  return PuyoColor::EMPTY;
    case TLColor::OJAMA:  return PuyoColor::OJAMA;
    case TLColor::RED:    return PuyoColor::RED;
    case TLColor::BLUE:   return PuyoColor::BLUE;
    case TLColor::YELLOW: return PuyoColor::YELLOW;
    case TLColor::GREEN:  return PuyoColor::GREEN;
    default: CHECK(false);
    }
}

TLColor toValidTLColor(TLColor c)
{
    // HACK(peria): Convert TL_UNKNOWN to RED to avoid out-of-range.
    // NOTE: We can use other colors insted, but use only RED to keep code simple.
    if (c == TLColor::UNKNOWN)
        return TLColor::RED;
    return c;
}

std::ostream& operator<<(std::ostream& os, TLColor c)
{
    switch (c) {
    case TLColor::EMPTY:  return (os << '.');
    case TLColor::RED:    return (os << 'R');
    case TLColor::BLUE:   return (os << 'B');
    case TLColor::YELLOW: return (os << 'Y');
    case TLColor::GREEN:  return (os << 'G');
    case TLColor::OJAMA:  return (os << 'O');
    case TLColor::UNKNOWN:return (os << '?');
    }

    return (os << '?');
}

} // namespace test_lockit
