#ifndef CORE_ALGORITHM_PATTERN_MATCHER_H_
#define CORE_ALGORITHM_PATTERN_MATCHER_H_

#include <glog/logging.h>

#include <algorithm>
#include <vector>

#include "base/base.h"
#include "base/small_int_set.h"
#include "core/algorithm/field_pattern.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/puyo_color.h"

struct PatternMatchResult {
    PatternMatchResult() : matched(false) {}
    PatternMatchResult(bool matched, FieldBits matchedBits, FieldBits allowedMatchedBits,
                       SmallIntSet unusedVariables) :
        matched(matched),
        matchedBits(matchedBits),
        allowedMatchedBits(allowedMatchedBits),
        unusedVariables(unusedVariables) {}

    friend bool operator==(const PatternMatchResult& lhs, const PatternMatchResult& rhs)
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

struct ComplementResult{
    ComplementResult() {}
    ComplementResult(bool success,
                     const PatternMatchResult matchedResult,
                     int numFilledUnusedVariables,
                     const CoreField& complementedField,
                     const ColumnPuyoList& cpl) :
        success(success),
        matchedResult(matchedResult),
        numFilledUnusedVariables(numFilledUnusedVariables),
        complementedField(complementedField),
        complementedPuyoList(cpl) {}

    bool success = false;
    PatternMatchResult matchedResult;
    int numFilledUnusedVariables = 0;
    CoreField complementedField;
    ColumnPuyoList complementedPuyoList;
};

class PatternMatcher {
public:
    PatternMatcher();

    // If |ignoreMustVar| is true, don't check the existence.
    PatternMatchResult match(const FieldPattern&, const CoreField&, bool ignoresMustVar = false);

    ComplementResult complement(const FieldPattern&, const CoreField&,
                                int numAllowingFillingUnusedVariables = 0);

private:
    PatternMatchResult matchInternal(const FieldPattern& fieldPattern,
                                     const CoreField& field,
                                     bool ignoresMustVar,
                                     BitField* complemented,
                                     PuyoColor env[]);
};

#endif
