#include "color.h"

#include <glog/logging.h>

namespace test_lockit {

PuyoColor toValidPuyoColor(PuyoColor c)
{
    // HACK(peria): Convert TL_UNKNOWN to RED to avoid out-of-range.
    // NOTE: We can use other colors insted, but use only RED to keep code simple.
    if (c == PuyoColor::IRON)
        return PuyoColor::RED;
    return c;
}

} // namespace test_lockit
