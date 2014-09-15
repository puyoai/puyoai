#include "core/real_color.h"

#include <glog/logging.h>

RealColor toRealColor(char c)
{
    switch (c) {
    case ' ': return RealColor::RC_EMPTY;
    case 'O': return RealColor::RC_OJAMA;
    case 'R': return RealColor::RC_RED;
    case 'B': return RealColor::RC_BLUE;
    case 'Y': return RealColor::RC_YELLOW;
    case 'G': return RealColor::RC_GREEN;
    case 'P': return RealColor::RC_PURPLE;
    }

    DCHECK(false) << "Unknown char: " << c;
    return RealColor::RC_EMPTY;
}

char toChar(RealColor rc, bool usesCapital)
{
    switch (rc) {
    case RealColor::RC_EMPTY:  return ' ';
    case RealColor::RC_OJAMA:  return usesCapital ? 'O' : 'o';
    case RealColor::RC_RED:    return usesCapital ? 'R' : 'r';
    case RealColor::RC_BLUE:   return usesCapital ? 'B' : 'b';
    case RealColor::RC_YELLOW: return usesCapital ? 'Y' : 'y';
    case RealColor::RC_GREEN:  return usesCapital ? 'G' : 'g';
    case RealColor::RC_PURPLE: return usesCapital ? 'P' : 'p';
    }

    CHECK(false) << "Unknown RealColor: " << static_cast<int>(rc);
    return '-';
}

const char* toString(RealColor rc)
{
    switch (rc) {
    case RealColor::RC_EMPTY:
        return "空";
    case RealColor::RC_OJAMA:
        return "邪";
    case RealColor::RC_RED:
        return "赤";
    case RealColor::RC_BLUE:
        return "青";
    case RealColor::RC_YELLOW:
        return "黄";
    case RealColor::RC_GREEN:
        return "緑";
    case RealColor::RC_PURPLE:
        return "紫";
    }

    return "不";
}

std::ostream& operator<<(std::ostream& os, RealColor rc)
{
    return os << static_cast<int>(rc);
}
