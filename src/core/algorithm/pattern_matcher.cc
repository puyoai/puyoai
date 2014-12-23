#include "core/algorithm/pattern_matcher.h"

BijectionMatcher::BijectionMatcher()
{
    for (int i = 0; i < 4; ++i)
        colors_[i] = PuyoColor::EMPTY;
    for (int i = 0; i < NUM_PUYO_COLORS; ++i)
        chars_[i] = ' ';
}

bool BijectionMatcher::match(char v, PuyoColor c)
{
    DCHECK('A' <= v && v <= 'D') << v;
    int idx = v - 'A';

    if (colors_[idx] == c)
        return true;

    if (colors_[idx] == PuyoColor::EMPTY && chars_[ordinal(c)] == ' ') {
        colors_[idx] = c;
        chars_[ordinal(c)] = v;
        return true;
    }

    return false;
}
