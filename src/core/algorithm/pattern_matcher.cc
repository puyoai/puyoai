#include "core/algorithm/pattern_matcher.h"

#include <cctype>

#include "core/algorithm/field_pattern.h"
#include "core/core_field.h"

using namespace std;

PatternMatcher::PatternMatcher() :
    seen_ {}
{
    for (int i = 0; i < 26; ++i) {
        map_[i] = PuyoColor::WALL;
    }
}

bool PatternMatcher::checkNeighborsForCompletion(const FieldPattern& pattern, const CoreField& cf) const
{
    // Check the neighbors.
    for (int x = 1; x <= 6; ++x) {
        int h = pattern.height(x);
        for (int y = 1; y <= h; ++y) {
            char c = pattern.variable(x, y);
            if (!(pattern.type(x, y) == PatternType::VAR || pattern.type(x, y) == PatternType::MUST_VAR))
                continue;
            if (!checkCell(c, pattern.type(x, y + 1), pattern.variable(x, y + 1), cf.color(x, y + 1)))
                return false;
            if (!checkCell(c, pattern.type(x, y - 1), pattern.variable(x, y - 1), cf.color(x, y - 1)))
                return false;
            if (!checkCell(c, pattern.type(x + 1, y), pattern.variable(x + 1, y), cf.color(x + 1, y)))
                return false;
            if (!checkCell(c, pattern.type(x - 1, y), pattern.variable(x - 1, y), cf.color(x - 1, y)))
                return false;
        }
    }

    return true;
}

bool PatternMatcher::checkCell(char currentVar,
                               PatternType neighborType,
                               char neighborVar,
                               PuyoColor neighborColor) const
{
    DCHECK('A' <= currentVar && currentVar <= 'Z') << currentVar;

    // This case should be already processed.
    if (currentVar == neighborVar)
        return true;

    if (neighborColor == PuyoColor::OJAMA || neighborColor == PuyoColor::WALL)
        return true;

    switch (neighborType) {
    case PatternType::NONE:
    case PatternType::ALLOW_FILLING_IRON:
        if (map(currentVar) == neighborColor)
            return false;
        break;
    case PatternType::ALLOW_VAR:
        DCHECK('A' <= neighborVar && neighborVar <= 'Z');
        if (currentVar != neighborVar && map(currentVar) == neighborColor)
            return false;
        break;
    case PatternType::ANY:
        // If neighbor is '*', we don't care what color the cell has.
        return true;
    default:
        if (map(currentVar) == map(neighborVar) && isSet(currentVar))
            return false;
        break;
    }

    return true;
}

bool PatternMatcher::fillUnusedVariableColors(const FieldPattern& pattern,
                                              const CoreField& field,
                                              bool needsNeighborCheck,
                                              SmallIntSet unusedVariables,
                                              ColumnPuyoList* cpl)
{
    if (unusedVariables.isEmpty()) {
        if (needsNeighborCheck && !checkNeighborsForCompletion(pattern, field))
            return false;
        return complementInternal(pattern, field, cpl);
    }

    char c = unusedVariables.smallest() + 'A';
    unusedVariables.removeSmallest();
    for (PuyoColor pc : NORMAL_PUYO_COLORS) {
        forceSet(c, pc);
        if (fillUnusedVariableColors(pattern, field, needsNeighborCheck, unusedVariables, cpl))
            return true;
    }
    return false;
}

bool PatternMatcher::complementInternal(const FieldPattern& pattern,
                                        const CoreField& field,
                                        ColumnPuyoList* cpl)
{
    cpl->clear();

    int currentHeights[FieldConstant::MAP_WIDTH] {
        0, field.height(1), field.height(2), field.height(3),
        field.height(4), field.height(5), field.height(6), 0
    };

    for (int x = 1; x <= 6; ++x) {
        int h = pattern.height(x);
        for (int y = 1; y <= h; ++y) {
            if (pattern.type(x, y) == PatternType::ALLOW_FILLING_IRON) {
                if (!(field.isColor(x, y, PuyoColor::EMPTY) || field.isColor(x, y, PuyoColor::IRON)))
                    continue;
                if (!cpl->add(x, PuyoColor::IRON))
                    return false;
                ++currentHeights[x];
                continue;
            }

            if (pattern.type(x, y) == PatternType::ALLOW_VAR)
                continue;

            if (!(pattern.type(x, y) == PatternType::VAR || pattern.type(x, y) == PatternType::MUST_VAR)) {
                if (field.color(x, y) == PuyoColor::EMPTY)
                    return false;
                continue;
            }

            char c = pattern.variable(x, y);
            if (!isSet(c))
                return false;
            if (pattern.type(x, y) == PatternType::MUST_VAR) {
                if (!isNormalColor(field.color(x, y)))
                    return false;
            }
            if (field.color(x, y) == PuyoColor::EMPTY && currentHeights[x] + 1 == y) {
                if (!cpl->add(x, map(c)))
                    return false;
                ++currentHeights[x];
            } else if (field.color(x, y) == PuyoColor::IRON) {
                if (!cpl->add(x, map(c)))
                    return false;
            }
        }
    }

    return true;
}

PatternMatchResult PatternMatcher::match(const FieldPattern& pattern,
                                         const CoreField& cf,
                                         bool ignoresMustVar)
{
    // First, make a map from char to PuyoColor.
    FieldBits matchedBits;
    FieldBits allowedMatchedBits;

    // First, create a env (char -> PuyoColor)
    for (int x = 1; x <= 6; ++x) {
        int h = cf.height(x);
        for (int y = 1; y <= h; ++y) {
            char c = pattern.variable(x, y);

            if (!(pattern.type(x, y) == PatternType::VAR || pattern.type(x, y) == PatternType::MUST_VAR))
                continue;

            PuyoColor pc = cf.color(x, y);
            if (pc == PuyoColor::EMPTY) {
                if (!ignoresMustVar && pattern.type(x, y) == PatternType::MUST_VAR)
                    return PatternMatchResult();
                continue;
            }

            // place holder.
            if (pc == PuyoColor::IRON)
                continue;

            if (!isNormalColor(pc))
                return PatternMatchResult();

            matchedBits.set(x, y);

            if (!isSet(c)) {
                set(c, pc);
                continue;
            }

            if (map(c) != pc)
                return PatternMatchResult();
        }
    }

    // Check the neighbors.
    for (int x = 1; x <= 6; ++x) {
        int h = pattern.height(x);
        for (int y = 1; y <= h; ++y) {
            char c = pattern.variable(x, y);
            if (pattern.type(x, y) == PatternType::NONE)
                continue;
            if (pattern.type(x, y) == PatternType::ANY)
                continue;
            if (pattern.type(x, y) == PatternType::ALLOW_FILLING_IRON)
                continue;
            if (pattern.type(x, y) == PatternType::ALLOW_VAR) {
                char uv = std::toupper(pattern.variable(x, y));
                if (isSet(uv) && map(uv) == cf.color(x, y))
                    allowedMatchedBits.set(x, y);
                continue;
            }

            setSeen(c);

            if (!ignoresMustVar && pattern.type(x, y) == PatternType::MUST_VAR && cf.color(x, y) == PuyoColor::EMPTY)
                return PatternMatchResult();

            DCHECK(pattern.type(x, y) == PatternType::VAR || pattern.type(x, y) == PatternType::MUST_VAR);

            // Check neighbors.
            if (!checkCell(c, pattern.type(x, y + 1), pattern.variable(x, y + 1), cf.color(x, y + 1)))
                return PatternMatchResult();
            if (!checkCell(c, pattern.type(x, y - 1), pattern.variable(x, y - 1), cf.color(x, y - 1)))
                return PatternMatchResult();
            if (!checkCell(c, pattern.type(x + 1, y), pattern.variable(x + 1, y), cf.color(x + 1, y)))
                return PatternMatchResult();
            if (!checkCell(c, pattern.type(x - 1, y), pattern.variable(x - 1, y), cf.color(x - 1, y)))
                return PatternMatchResult();
        }
    }

    SmallIntSet unusedVariables;
    for (char c = 'A'; c <= 'Z'; ++c) {
        if (isSeen(c) && !isSet(c))
            unusedVariables.set(c - 'A');
    }

    return PatternMatchResult(true, matchedBits, allowedMatchedBits, unusedVariables);
}

ComplementResult PatternMatcher::complement(const FieldPattern& pattern,
                                            const CoreField& field,
                                            int numAllowingFillingUnusedVariables)
{
    PatternMatchResult result = match(pattern, field, false);
    if (!result.matched)
        return ComplementResult();

    if (result.unusedVariables.size() > numAllowingFillingUnusedVariables)
        return ComplementResult();

    ColumnPuyoList cpl;
    bool ok = fillUnusedVariableColors(pattern, field, !result.unusedVariables.isEmpty(), result.unusedVariables, &cpl);
    int filled = std::min(static_cast<int>(result.unusedVariables.size()),
                          numAllowingFillingUnusedVariables);
    return ComplementResult(ok, result, filled, cpl);
}
