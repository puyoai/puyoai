#ifndef CORE_ALGORITHM_PATTERN_MATCHER_H_
#define CORE_ALGORITHM_PATTERN_MATCHER_H_

#include <tuple>
#include <vector>

#include "core/puyo_color.h"
#include "core/algorithm/field_pattern.h"

class CoreField;
class FieldPattern;

struct PatternMatchResult {
    PatternMatchResult() : matched(false), score(0), count(0), allowedCount(0) {}
    PatternMatchResult(bool matched, double score, int count, int allowedCount) :
        matched(matched), score(score), count(count), allowedCount(allowedCount) {}
    PatternMatchResult(bool matched, double score, int count, int allowedCount,
                       std::vector<char> unusedVariables) :
        matched(matched), score(score), count(count), allowedCount(allowedCount),
        unusedVariables(std::move(unusedVariables)) {}

    friend bool operator==(const PatternMatchResult& lhs, const PatternMatchResult& rhs)
    {
        return std::tie(lhs.matched, lhs.score, lhs.count, lhs.allowedCount, lhs.unusedVariables) ==
            std::tie(rhs.matched, rhs.score, rhs.count, rhs.allowedCount, rhs.unusedVariables);
    }

    bool matched;
    double score;
    int count;
    int allowedCount;
    std::vector<char> unusedVariables;
};

class PatternMatcher {
public:
    PatternMatcher();

    // If |ignoreMustVar| is true, don't check the existence.
    PatternMatchResult match(const FieldPattern&, const CoreField&, bool ignoresMustVar = false);
    bool checkNeighborsForCompletion(const FieldPattern&, const CoreField&) const;

    PuyoColor map(char var) const
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        return map_[var - 'A'];
    }

    bool isSet(char var) const
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        return map_[var - 'A'] != PuyoColor::WALL;
    }

    void forceSet(char var, PuyoColor pc)
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        map_[var - 'A'] = pc;
    }

private:
    PuyoColor set(char var, PuyoColor pc)
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        return map_[var - 'A'] = pc;
    }

    bool isSeen(char var) const { return seen_[var - 'A']; }

    void setSeen(char var)
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        seen_[var - 'A'] = true;
    }

    bool checkCell(char currentVar, PatternType neighborType, char neighborVar, PuyoColor neighborColor) const;

    PuyoColor map_[26];
    bool seen_[26];
};

#endif
