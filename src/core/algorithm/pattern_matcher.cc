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

    // If neighbor is '*', we don't care what color the cell has.
    if (neighborType == PatternType::ANY)
        return true;

    if (neighborColor == PuyoColor::OJAMA || neighborColor == PuyoColor::WALL)
        return true;

    // This case should be already processed.
    if (currentVar == neighborVar)
        return true;

    switch (neighborType) {
    case PatternType::NONE:
    case PatternType::ALLOW_FILLING_OJAMA:
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
        CHECK(false) << "shoudn't be reached";
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
            if (pattern.type(x, y) == PatternType::ALLOW_FILLING_OJAMA) {
                if (field.color(x, y) != PuyoColor::EMPTY)
                    continue;
                if (!cpl->add(x, PuyoColor::OJAMA))
                    return false;
                ++currentHeights[x];
                continue;
            }

            if (pattern.type(x, y) == PatternType::ALLOW_FILLING_IRON) {
                if (field.color(x, y) != PuyoColor::EMPTY)
                    continue;
                if (!cpl->add(x, PuyoColor::IRON))
                    return false;
                ++currentHeights[x];
                continue;
            }

            if (pattern.type(x, y) == PatternType::ALLOW_VAR)
                continue;

            if (!(pattern.type(x, y) == PatternType::VAR || pattern.type(x, y) == PatternType::MUST_VAR)) {
                if (field.color(x, y) == PuyoColor::EMPTY) {
                    return false;
                }
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
            }
        }
    }

    return true;
}
