#include "core/algorithm/bit_pattern_matcher.h"

#include "base/small_int_set.h"
#include "core/algorithm/bit_field_pattern.h"
#include "core/bit_field.h"
#include "core/puyo_color.h"

#include <iostream>

using namespace std;

BitPatternMatchResult BitPatternMatcher::match(const BitFieldPattern& bitFieldPattern,
                                               const BitField& field,
                                               bool /*ignoresMustVar*/)
{
    SmallIntSet unusedVariables;

    FieldBits matchedBits;

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
                return BitPatternMatchResult();
            }

            matchedBits.setAll(bits);
            found = c;
        }

        // OJAMA is matched?  (OK for IRON, since it's a place holder.)
        if (!FieldBits(pat.varBits & field.bits(PuyoColor::OJAMA)).isEmpty())
            return BitPatternMatchResult();

        envs_[i] = found;
        if (found == PuyoColor::EMPTY) {
            DCHECK('A' <= pat.var && pat.var <= 'Z') << pat.var;
            unusedVariables.set(pat.var - 'A');
        }
    }

    // Check the neighbors.
    // If the neighbor of pattern has the same color
    FieldBits allowedMatchedBits;
    for (size_t i = 0; i < bitFieldPattern.patterns().size(); ++i) {
        if (envs_[i] == PuyoColor::EMPTY)
            continue;
        const BitFieldPattern::Pattern& pat = bitFieldPattern.pattern(i);
        FieldBits edge = pat.varBits.expandEdge().notmask(pat.varBits) & field.bits(envs_[i]);
        if (!edge.notmask(pat.allowVarBits).notmask(bitFieldPattern.anyPatternBits()).isEmpty())
            return BitPatternMatchResult();

        FieldBits matched = edge.mask(pat.allowVarBits);
        allowedMatchedBits.setAll(matched);
    }

    return BitPatternMatchResult(true, matchedBits, allowedMatchedBits, unusedVariables);
}
