#include "recognition/recognition_color.h"

#include <glog/logging.h>

RealColor toRealColor(RecognitionColor c)
{
    switch (c) {
    case RecognitionColor::RED:      return RealColor::RC_RED;
    case RecognitionColor::BLUE:     return RealColor::RC_BLUE;
    case RecognitionColor::YELLOW:   return RealColor::RC_YELLOW;
    case RecognitionColor::GREEN:    return RealColor::RC_GREEN;
    case RecognitionColor::PURPLE:   return RealColor::RC_PURPLE;
    case RecognitionColor::EMPTY:    return RealColor::RC_EMPTY;
    case RecognitionColor::OJAMA:    return RealColor::RC_OJAMA;
    case RecognitionColor::ZENKESHI: return RealColor::RC_EMPTY;
    }

    CHECK(false) << "Unknown color";
    return RealColor::RC_EMPTY;
}
