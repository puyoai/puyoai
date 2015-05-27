#ifndef CORE_ALGORITHM_PATTERN_MATCH_RESULT_H_
#define CORE_ALGORITHM_PATTERN_MATCH_RESULT_H_

#include <tuple>

#include "base/small_int_set.h"

struct PatternMatchResult {
    PatternMatchResult() : matched(false), score(0), count(0), allowedCount(0) {}
    PatternMatchResult(bool matched, double score, int count, int allowedCount) :
        matched(matched), score(score), count(count), allowedCount(allowedCount) {}
    PatternMatchResult(bool matched, double score, int count, int allowedCount,
                       SmallIntSet unusedVariables) :
        matched(matched), score(score), count(count), allowedCount(allowedCount),
        unusedVariables(unusedVariables) {}

    friend bool operator==(const PatternMatchResult& lhs, const PatternMatchResult& rhs)
    {
        return std::tie(lhs.matched, lhs.score, lhs.count, lhs.allowedCount, lhs.unusedVariables) ==
            std::tie(rhs.matched, rhs.score, rhs.count, rhs.allowedCount, rhs.unusedVariables);
    }

    bool matched;
    double score;
    int count;
    int allowedCount;
    SmallIntSet unusedVariables; // 'A' -> 0, 'B' -> 1, ...
};

#endif // CORE_ALGORITHM_PATTERN_MATCH_RESULT_H_
