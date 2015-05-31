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
                     const ColumnPuyoList& cpl) :
        success(success),
        matchedResult(matchedResult),
        numFilledUnusedVariables(numFilledUnusedVariables),
        complementedPuyoList(cpl) {}

    bool success = false;
    PatternMatchResult matchedResult;
    int numFilledUnusedVariables = 0;
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
    bool checkNeighborsForCompletion(const FieldPattern&, const CoreField&) const;
    bool fillUnusedVariableColors(const FieldPattern&,
                                  const CoreField&,
                                  bool needsNeighborCheck,
                                  SmallIntSet unusedVariables,
                                  ColumnPuyoList*);
    bool complementInternal(const FieldPattern&,
                            const CoreField&,
                            ColumnPuyoList*);

    PuyoColor map_[26];
    bool seen_[26];
};

#endif
