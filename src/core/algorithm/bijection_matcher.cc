#include "core/algorithm/bijection_matcher.h"

#include "core/algorithm/field_pattern.h"
#include "core/core_field.h"

BijectionMatcher::BijectionMatcher()
{
    for (int i = 0; i < 4; ++i)
        colors_[i] = PuyoColor::EMPTY;
    for (int i = 0; i < NUM_PUYO_COLORS; ++i)
        chars_[i] = ' ';
}

bool BijectionMatcher::match(const FieldPattern& pattern, const CoreField& cf)
{
    // Check heights first, since this is fast.
    for (int x = 1; x <= 6; ++x) {
        if (cf.height(x) != pattern.height(x))
            return false;
    }

    for (int x = 1; x <= 6; ++x) {
        int h = pattern.height(x);
        for (int y = 1; y <= h; ++y) {
            if (!match(pattern.variable(x, y), cf.color(x, y)))
                return false;
        }
    }

    return true;
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

