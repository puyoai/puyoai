#include "core/algorithm/pattern_matcher.h"

#include "core/algorithm/pattern_field.h"
#include "core/core_field.h"

PatternMatcher::PatternMatcher()
{
    for (int i = 0; i < 26; ++i)
        map_[i] = PuyoColor::WALL;
}

PatternMatchResult PatternMatcher::match(const PatternField& pf, const CoreField& cf)
{
    // First, make a map from char to PuyoColor.
    int matchCount = 0;
    int matchAllowedCount = 0;
    double matchScore = 0;

    // First, create a env (char -> PuyoColor)
    for (int x = 1; x <= 6; ++x) {
        int h = cf.height(x);
        for (int y = 1; y <= h; ++y) {
            char c = pf.variable(x, y);
            if (c < 'A' || 'Z' < c)
                continue;

            PuyoColor pc = cf.color(x, y);
            if (pc == PuyoColor::EMPTY)
                continue;

            if (!isNormalColor(pc))
                return PatternMatchResult(false, 0, 0, 0);

            matchCount += 1;
            matchScore += pf.score(x, y);

            if (!isSet(c)) {
                set(c, pc);
                continue;
            }

            if (map(c) != pc)
                return PatternMatchResult(false, 0, 0, 0);
        }
    }

    // Check the neighbors.
    for (int x = 1; x <= 6; ++x) {
        int h = pf.height(x);
        for (int y = 1; y <= h; ++y) {
            char c = pf.variable(x, y);
            if (c == '*' || c == ' ' || c == '.')
                continue;

            if ('a' <= c && c <= 'z') {
                char uv = std::toupper(pf.variable(x, y));
                if (isSet(uv) && map(uv) == cf.color(x, y)) {
                    ++matchAllowedCount;
                }
                continue;
            }

            // Check neighbors.
            if (!checkCell(c, pf.variable(x, y + 1), cf.color(x, y + 1)))
                return PatternMatchResult(false, 0, 0, 0);
            if (!checkCell(c, pf.variable(x, y - 1), cf.color(x, y - 1)))
                return PatternMatchResult(false, 0, 0, 0);
            if (!checkCell(c, pf.variable(x + 1, y), cf.color(x + 1, y)))
                return PatternMatchResult(false, 0, 0, 0);
            if (!checkCell(c, pf.variable(x - 1, y), cf.color(x - 1, y)))
                return PatternMatchResult(false, 0, 0, 0);
        }
    }

    return PatternMatchResult(true, matchScore, matchCount, matchAllowedCount);
}

inline
bool PatternMatcher::checkCell(char currentVar, char neighborVar, PuyoColor neighborColor)
{
    DCHECK('A' <= currentVar && currentVar <= 'Z') << currentVar;

    // If neighbor is '*', we don't care what color the cell has.
    if (neighborVar == '*')
        return true;

    if (neighborColor == PuyoColor::OJAMA || neighborColor == PuyoColor::WALL)
        return true;

    // This case should be already processed.
    if (currentVar == neighborVar)
        return true;

    if (neighborVar == '.' || neighborVar == ' ') {
        if (map(currentVar) == neighborColor)
            return false;
    } else if ('a' <= neighborVar && neighborVar <= 'z') {
        if (currentVar != std::toupper(neighborVar) && map(currentVar) == neighborColor)
            return false;
    } else {
        if (map(currentVar) == map(neighborVar) && isSet(currentVar))
            return false;
    }

    return true;
}
