#include "core/algorithm/bit_pattern_matcher.h"

#include "base/small_int_set.h"
#include "core/algorithm/bit_field_pattern.h"
#include "core/bit_field.h"
#include "core/puyo_color.h"

#include <iostream>

using namespace std;

bool BitPatternMatcher::match(const BitFieldPattern& bitFieldPattern, const BitField& field)
{
    PuyoColor envs[26];
    SmallIntSet unusedVariables;

    int matchCount = 0;

    // First, create a env (char -> PuyoColor)
    for (size_t i = 0; i < bitFieldPattern.patterns().size(); ++i) {
        const BitFieldPattern::Pattern& pat = bitFieldPattern.pattern(i);
        PuyoColor found = PuyoColor::EMPTY;
        for (PuyoColor c : NORMAL_PUYO_COLORS) {
            FieldBits bits = pat.varBits & field.bits(c);
            if (bits.isEmpty())
                continue;
            if (found != PuyoColor::EMPTY) {
                // found multiple colors for one variable.
                return false;
            }
            matchCount += bits.popcount();
            found = c;
        }

        // OJAMA is matched?  (OK for IRON, since it's a place holder.)
        if (!FieldBits(pat.varBits & field.bits(PuyoColor::OJAMA)).isEmpty())
            return false;

        envs[i] = found;
        if (found == PuyoColor::EMPTY) {
            DCHECK('A' <= pat.var && pat.var <= 'Z') << pat.var;
            unusedVariables.set(pat.var - 'A');
        }
    }

    // Check the neighbors.
    // If the neighbor of pattern has the same color
    int allowMatchCount = 0;
    for (size_t i = 0; i < bitFieldPattern.patterns().size(); ++i) {
        if (envs[i] == PuyoColor::EMPTY)
            continue;
        const BitFieldPattern::Pattern& pat = bitFieldPattern.pattern(i);
        FieldBits edge = pat.varBits.expandEdge().notmask(pat.varBits) & field.bits(envs[i]);
        if (!edge.notmask(pat.allowVarBits).notmask(bitFieldPattern.anyPatternBits()).isEmpty())
            return false;
        allowMatchCount += edge.mask(pat.allowVarBits).popcount();
    }

    return true;
}
