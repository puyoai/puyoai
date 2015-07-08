#ifndef TEST_LOCKIT_COLOR_H_
#define TEST_LOCKIT_COLOR_H_

#include "core/puyo_color.h"

namespace test_lockit {

// PuyoColor for test_lockit. When all puyo color is converted to this enum,
// the real PuyoColor should be used.
enum TLColor {
    EMPTY = 0,
    RED = 1,
    BLUE = 2,
    YELLOW = 3,
    GREEN = 4,
    OJAMA = 9,

    // TODO(peria): Drop this item.
    UNKNOWN = 10,
};

inline bool isNormalTLColor(TLColor color)
{
    switch (color) {
    case TLColor::RED:
    case TLColor::BLUE:
    case TLColor::YELLOW:
    case TLColor::GREEN:
        return true;
    default:
        return false;
    }
}

// Color coverter
TLColor toTLColor(PuyoColor pc);
PuyoColor toPuyoColor(TLColor c);

TLColor toValidTLColor(TLColor c);

}  // namespace test_lockit

#endif
