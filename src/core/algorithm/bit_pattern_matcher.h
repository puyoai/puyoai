#ifndef CORE_ALGORITHM_BIT_PATTERN_MATCHER_H_
#define CORE_ALGORITHM_BIT_PATTERN_MATCHER_H_

#include "core/column_puyo_list.h"
#include "core/field_bits.h"

class BitFieldPattern;
class BitField;

struct BitPatternMatchResult {
    BitPatternMatchResult() : matched(false) {}

    BitPatternMatchResult(bool matched, FieldBits matchedBits, FieldBits allowedMatchedBits, SmallIntSet unusedVariables) :
        matched(matched), matchedBits(matchedBits), allowedMatchedBits(allowedMatchedBits), unusedVariables(unusedVariables) {}

    friend bool operator==(const BitPatternMatchResult& lhs, const BitPatternMatchResult& rhs)
    {
        if (lhs.matched != rhs.matched)
            return false;
        if (lhs.matchedBits != rhs.matchedBits)
            return false;
        if (lhs.allowedMatchedBits != rhs.allowedMatchedBits)
            return false;
        if (lhs.unusedVariables != rhs.unusedVariables)
            return false;
        return true;
    }

    bool matched;
    FieldBits matchedBits;
    FieldBits allowedMatchedBits;
    SmallIntSet unusedVariables; // 'A' -> 0, 'B' -> 1, ...
};

struct BitComplementResult {
    bool matched = false;
    int numFilledUnusedVariables = 0;
    ColumnPuyoList complementedPuyoList;
};

class BitPatternMatcher {
public:
    BitPatternMatchResult match(const BitFieldPattern&, const BitField&, bool ignoresMustVar = false);

private:
    PuyoColor envs_[26] {};
};

#endif // CORE_ALGORITHM_BIT_PATTERN_MATCHER_H_
