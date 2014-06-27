#ifndef CORE_REAL_COLOR_H_
#define CORE_REAL_COLOR_H_

// TODO(mayah): Consider use enum class.
enum RealColor {
    RC_EMPTY,
    RC_OJAMA,
    RC_RED,
    RC_BLUE,
    RC_YELLOW,
    RC_GREEN,
    RC_PURPLE,
};

const int NUM_REAL_COLORS = 8;

RealColor realColorOf(char c);
const char* toString(RealColor);

inline bool isNormalColor(RealColor rc)
{
    switch (rc) {
    case RealColor::RC_RED:
    case RealColor::RC_BLUE:
    case RealColor::RC_YELLOW:
    case RealColor::RC_GREEN:
    case RealColor::RC_PURPLE:
        return true;
    default:
        return false;
    }
}

#endif
