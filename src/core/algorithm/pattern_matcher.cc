#include "core/algorithm/pattern_matcher.h"

#include "core/algorithm/pattern_field.h"
#include "core/core_field.h"

PatternMatcher::PatternMatcher()
{
    for (int i = 0; i < 26; ++i)
        map_[i] = PuyoColor::WALL;
}

PatternMatchResult PatternMatcher::match(const PatternField& pf, const CoreField& cf, bool ignoresMustVar)
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
            if (pf.type(x, y) == PatternType::MUST_EMPTY) {
                if (cf.color(x, y) != PuyoColor::EMPTY)
                    return PatternMatchResult(false, 0, 0, 0);
                continue;
            }
            if (pf.type(x, y) != PatternType::VAR && pf.type(x, y) != PatternType::MUST_VAR)
                continue;

            PuyoColor pc = cf.color(x, y);
            if (pc == PuyoColor::EMPTY) {
                if (!ignoresMustVar && pf.type(x, y) == PatternType::MUST_VAR)
                    return PatternMatchResult(false, 0, 0, 0);
                continue;
            }

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
            if (pf.type(x, y) == PatternType::NONE || pf.type(x, y) == PatternType::ANY)
                continue;

            if (pf.type(x, y) == PatternType::ALLOW_VAR) {
                char uv = std::toupper(pf.variable(x, y));
                if (isSet(uv) && map(uv) == cf.color(x, y)) {
                    ++matchAllowedCount;
                }
                continue;
            }

            if (!ignoresMustVar && pf.type(x, y) == PatternType::MUST_VAR && cf.color(x, y) == PuyoColor::EMPTY)
                return PatternMatchResult(false, 0, 0, 0);

            DCHECK(pf.type(x, y) == PatternType::VAR || pf.type(x, y) == PatternType::MUST_VAR);

            // Check neighbors.
            if (!checkCell(c, pf.type(x, y + 1), pf.variable(x, y + 1), cf.color(x, y + 1)))
                return PatternMatchResult(false, 0, 0, 0);
            if (!checkCell(c, pf.type(x, y - 1), pf.variable(x, y - 1), cf.color(x, y - 1)))
                return PatternMatchResult(false, 0, 0, 0);
            if (!checkCell(c, pf.type(x + 1, y), pf.variable(x + 1, y), cf.color(x + 1, y)))
                return PatternMatchResult(false, 0, 0, 0);
            if (!checkCell(c, pf.type(x - 1, y), pf.variable(x - 1, y), cf.color(x - 1, y)))
                return PatternMatchResult(false, 0, 0, 0);
        }
    }

    return PatternMatchResult(true, matchScore, matchCount, matchAllowedCount);
}

inline
bool PatternMatcher::checkCell(char currentVar, PatternType neighborType, char neighborVar, PuyoColor neighborColor)
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

    if (neighborType == PatternType::NONE) {
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
