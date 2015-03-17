#include "core/algorithm/pattern_matcher.h"

#include <cctype>

#include "core/algorithm/field_pattern.h"
#include "core/core_field.h"

using namespace std;

PatternMatcher::PatternMatcher()
{
    for (int i = 0; i < 26; ++i) {
        map_[i] = PuyoColor::WALL;
        seen_[i] = false;
    }
}

PatternMatchResult PatternMatcher::match(const FieldPattern& pattern, const CoreField& cf, bool ignoresMustVar)
{
    // First, make a map from char to PuyoColor.
    int matchCount = 0;
    int matchAllowedCount = 0;
    double matchScore = 0;

    // First, create a env (char -> PuyoColor)
    for (int x = 1; x <= 6; ++x) {
        int h = cf.height(x);
        for (int y = 1; y <= h; ++y) {
            char c = pattern.variable(x, y);

            if (pattern.type(x, y) == PatternType::MUST_EMPTY) {
                if (cf.color(x, y) != PuyoColor::EMPTY)
                    return PatternMatchResult();
                continue;
            }

            if (pattern.type(x, y) == PatternType::ALLOW_FILLING_OJAMA)
                continue;
            if (pattern.type(x, y) == PatternType::ALLOW_FILLING_IRON)
                continue;

            if (!(pattern.type(x, y) == PatternType::VAR || pattern.type(x, y) == PatternType::MUST_VAR))
                continue;

            PuyoColor pc = cf.color(x, y);
            if (pc == PuyoColor::EMPTY) {
                if (!ignoresMustVar && pattern.type(x, y) == PatternType::MUST_VAR)
                    return PatternMatchResult();
                continue;
            }

            if (!isNormalColor(pc))
                return PatternMatchResult();

            matchCount += 1;
            matchScore += pattern.score(x, y);

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
            if (pattern.type(x, y) == PatternType::ALLOW_FILLING_OJAMA)
                continue;
            if (pattern.type(x, y) == PatternType::ALLOW_FILLING_IRON)
                continue;
            if (pattern.type(x, y) == PatternType::ALLOW_VAR) {
                char uv = std::toupper(pattern.variable(x, y));
                if (isSet(uv) && map(uv) == cf.color(x, y)) {
                    ++matchAllowedCount;
                }
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

    vector<char> unusedVariables;
    for (char c = 'A'; c <= 'Z'; ++c) {
        if (isSeen(c) && !isSet(c))
            unusedVariables.push_back(c);
    }

    return PatternMatchResult(true, matchScore, matchCount, matchAllowedCount, std::move(unusedVariables));
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

inline
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
