#include "core/real_color.h"

#include <glog/logging.h>

using namespace std;

RealColor toRealColor(char c)
{
    switch (c) {
    case ' ': return RealColor::RC_EMPTY;
    case '#': return RealColor::RC_WALL;
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
    case RealColor::RC_WALL:   return '#';
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

string toString(RealColor rc)
{
    switch (rc) {
    case RealColor::RC_EMPTY:
        return "EMPTY";
    case RealColor::RC_WALL:
        return "WALL";
    case RealColor::RC_OJAMA:
        return "OJAMA";
    case RealColor::RC_RED:
        return "RED";
    case RealColor::RC_BLUE:
        return "BLUE";
    case RealColor::RC_YELLOW:
        return "YELLOW";
    case RealColor::RC_GREEN:
        return "GREEN";
    case RealColor::RC_PURPLE:
        return "PURPLE";
    }

    CHECK(false) << "Unknown RealColor: " << static_cast<int>(rc);
    return "UNKNOWN";
}

std::ostream& operator<<(std::ostream& os, RealColor rc)
{
    return os << toString(rc);
}
