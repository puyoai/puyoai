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

bool PatternMatcher::checkCell(char currentVar, PatternType neighborType, char neighborVar, PuyoColor neighborColor) const
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

    if (neighborType == PatternType::NONE || neighborType == PatternType::ALLOW_FILLING_OJAMA || neighborType == PatternType::ALLOW_FILLING_IRON) {
        if (map(currentVar) == neighborColor)
            return false;
    } else if (neighborType == PatternType::ALLOW_VAR) {
        DCHECK('A' <= neighborVar && neighborVar <= 'Z');
        if (currentVar != neighborVar && map(currentVar) == neighborColor)
            return false;
    } else {
        if (map(currentVar) == map(neighborVar) && isSet(currentVar))
            return false;
    }

    return true;
}
