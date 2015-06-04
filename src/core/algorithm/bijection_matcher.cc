#include "core/algorithm/bijection_matcher.h"

#include <glog/logging.h>

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
    FieldBits field13Bits = cf.bitField().field13Bits();
    FieldBits patternBits = pattern.patternBits();

    if (field13Bits != patternBits)
        return false;

    for (const auto& pat : pattern.patterns()) {
        bool found = false;
        for (PuyoColor c : NORMAL_PUYO_COLORS) {
            if ((pat.varBits & cf.bitField().bits(c)).isEmpty())
                continue;
            if (found)
                return false;
            found = true;
            if (!match(pat.var, c))
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
