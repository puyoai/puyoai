#include "core/algorithm/pattern_matcher.h"

#include <cctype>

#include "core/algorithm/field_pattern.h"
#include "core/core_field.h"

using namespace std;

PatternMatcher::PatternMatcher()
{
}

PatternMatchResult PatternMatcher::matchInternal(const FieldPattern& fieldPattern,
                                                 const CoreField& field,
                                                 bool ignoresMustVar,
                                                 BitField* complementedField,
                                                 PuyoColor envs[])
{
    if (!ignoresMustVar) {
        FieldBits emptyBits = field.bitField().bits(PuyoColor::EMPTY);
        if (!(fieldPattern.mustPatternBits() & emptyBits).isEmpty())
            return PatternMatchResult();
    }


    SmallIntSet unusedVariables;

    FieldBits matchedBits;

    // First, create a env (char -> PuyoColor)
    for (size_t i = 0; i < fieldPattern.patterns().size(); ++i) {
        const FieldPattern::Pattern& pat = fieldPattern.pattern(i);
        PuyoColor found = PuyoColor::EMPTY;
        for (PuyoColor c : NORMAL_PUYO_COLORS) {
            FieldBits bits = pat.varBits & field.bitField().bits(c);
            if (bits.isEmpty())
                continue;
            if (found != PuyoColor::EMPTY) {
                // found multiple colors for one variable.
                return PatternMatchResult();
            }

            matchedBits.setAll(bits);
            complementedField->setColorAll(pat.varBits, c);
            found = c;
        }

        // OJAMA is matched?  (OK for IRON, since it's a place holder.)
        if (!FieldBits(pat.varBits & field.bitField().bits(PuyoColor::OJAMA)).isEmpty())
            return PatternMatchResult();

        envs[i] = found;
        if (found == PuyoColor::EMPTY) {
            DCHECK('A' <= pat.var && pat.var <= 'Z') << pat.var;
            unusedVariables.set(pat.var - 'A');
        }
    }

    // Check the neighbors.
    // If the neighbor of pattern has the same color
    FieldBits allowedMatchedBits;
    for (size_t i = 0; i < fieldPattern.patterns().size(); ++i) {
        if (envs[i] == PuyoColor::EMPTY)
            continue;
        const FieldPattern::Pattern& pat = fieldPattern.pattern(i);
        FieldBits edge = pat.varBits.expandEdge().notmask(pat.varBits) & complementedField->bits(envs[i]);
        if (!edge.notmask(pat.allowVarBits).notmask(fieldPattern.anyPatternBits()).isEmpty())
            return PatternMatchResult();

        FieldBits matched = edge.mask(pat.allowVarBits);
        allowedMatchedBits.setAll(matched);
    }

    return PatternMatchResult(true, matchedBits, allowedMatchedBits, unusedVariables);
}

PatternMatchResult PatternMatcher::match(const FieldPattern& fieldPattern,
                                         const CoreField& field,
                                         bool ignoresMustVar)
{
    BitField complementedField(field.bitField());
    PuyoColor envs[26] {};
    return matchInternal(fieldPattern, field, ignoresMustVar, &complementedField, envs);
}

ComplementResult PatternMatcher::complement(const FieldPattern& fieldPattern,
                                            const CoreField& field,
                                            int numAllowingFillingUnusedVariables)
{
    DCHECK_LE(numAllowingFillingUnusedVariables, 1) << "Not supported";

    BitField complementedField(field.bitField());
    PuyoColor envs[26] {};

    PatternMatchResult result = matchInternal(fieldPattern, field, false, &complementedField, envs);
    if (!result.matched)
        return ComplementResult();

    if (result.unusedVariables.size() > numAllowingFillingUnusedVariables)
        return ComplementResult();

    // Complement unused variables.
    if (numAllowingFillingUnusedVariables > 0 && result.unusedVariables.size() > 0) {
        for (size_t i = 0; i < fieldPattern.patterns().size(); ++i) {
            if (envs[i] != PuyoColor::EMPTY)
                continue;

            bool ok = false;
            const FieldPattern::Pattern& pat = fieldPattern.pattern(i);
            for (PuyoColor c : NORMAL_PUYO_COLORS) {
                envs[i] = c;
                complementedField.setColorAll(pat.varBits, c);

                // Check neighbors.
                FieldBits edge = pat.varBits.expandEdge().notmask(pat.varBits) & complementedField.bits(envs[i]);
                if (!edge.notmask(pat.allowVarBits).notmask(fieldPattern.anyPatternBits()).isEmpty())
                    continue;

                ok = true;
                break;
            }

            if (!ok)
                return ComplementResult();
        }
    }

    // Complement IRON.
    complementedField.setColorAllIfEmpty(fieldPattern.ironPatternBits(), PuyoColor::IRON);
    FieldBits complementedBits = complementedField.differentBits(field.bitField());

    // If there are empty space below the complemented cell, it's not ok.
    if (!(_mm_srli_epi16(complementedBits, 1) & complementedField.bits(PuyoColor::EMPTY)).isEmpty())
        return ComplementResult();

    ColumnPuyoList cpl;
    bool ok = true;
    complementedBits.iterateBitPositions([&](int x, int y) {
        if (!cpl.add(x, complementedField.color(x, y)))
            ok = false;
    });

    if (!ok)
        return ComplementResult();

    return ComplementResult(true, result, result.unusedVariables.size(), CoreField(complementedField), cpl);
}
