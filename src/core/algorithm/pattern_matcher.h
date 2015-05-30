#ifndef CORE_ALGORITHM_PATTERN_MATCHER_H_
#define CORE_ALGORITHM_PATTERN_MATCHER_H_

#include <glog/logging.h>

#include <algorithm>
#include <tuple>
#include <vector>

#include "base/base.h"
#include "core/algorithm/field_pattern.h"
#include "core/algorithm/pattern_match_result.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/puyo_color.h"

class CoreField;

struct ComplementResult {
    explicit ComplementResult(bool success, int filled = 0) :
        success(success), numFilledUnusedVariables(filled) {}

    bool success = false;
    int numFilledUnusedVariables = 0;
};

class PatternMatcher {
public:
    PatternMatcher();

    // If |ignoreMustVar| is true, don't check the existence.
    template<typename ScoreCallback>
    PatternMatchResult match(const FieldPattern&, const CoreField&,
                             bool ignoresMustVar, ScoreCallback callback) NOINLINE_UNLESS_RELEASE;

    PatternMatchResult match(const FieldPattern& pattern, const CoreField& field, bool ignoresMustVar = false)
    {
        return match(pattern, field, ignoresMustVar, [](int /*x*/, int /*y*/, int /*score*/){});
    }

    // ScoreCallback must be a function-like: void f(int x, int y, double score).
    template<typename ScoreCallback>
    ComplementResult complement(const FieldPattern&,
                                const CoreField&,
                                int numAllowingFillingUnusedVariables,
                                ColumnPuyoList*,
                                ScoreCallback callback) NOINLINE_UNLESS_RELEASE;

    ComplementResult complement(const FieldPattern& fp, const CoreField& cf, int numAllowingFillingUnusedVariables,  ColumnPuyoList* cpl)
    {
        return complement(fp, cf, numAllowingFillingUnusedVariables, cpl, [](int /*x*/, int /*y*/, double /*score*/){});
    }

    ComplementResult complement(const FieldPattern& fp, const CoreField& cf, ColumnPuyoList* cpl)
    {
        return complement(fp, cf, 0, cpl, [](int /*x*/, int /*y*/, double /*score*/){});
    }

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

template<typename ScoreCallback>
PatternMatchResult PatternMatcher::match(const FieldPattern& pattern,
                                         const CoreField& cf,
                                         bool ignoresMustVar,
                                         ScoreCallback scoreCallback)
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

            if (!(pattern.type(x, y) == PatternType::VAR || pattern.type(x, y) == PatternType::MUST_VAR))
                continue;

            PuyoColor pc = cf.color(x, y);
            if (pc == PuyoColor::EMPTY) {
                if (!ignoresMustVar && pattern.type(x, y) == PatternType::MUST_VAR)
                    return PatternMatchResult();
                continue;
            }

            // place holder.
            if (pc == PuyoColor::IRON)
                continue;

            if (!isNormalColor(pc))
                return PatternMatchResult();

            matchCount += 1;
            matchScore += pattern.score(x, y);
            scoreCallback(x, y, pattern.score(x, y));

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

    SmallIntSet unusedVariables;
    for (char c = 'A'; c <= 'Z'; ++c) {
        if (isSeen(c) && !isSet(c))
            unusedVariables.set(c - 'A');
    }

    return PatternMatchResult(true, matchScore, matchCount, matchAllowedCount, unusedVariables);
}

template<typename ScoreCallback>
ComplementResult PatternMatcher::complement(const FieldPattern& pattern,
                                            const CoreField& field,
                                            int numAllowingFillingUnusedVariables,
                                            ColumnPuyoList* cpl,
                                            ScoreCallback scoreCallback)
{
    DCHECK_EQ(cpl->size(), 0) << "result must be empty";

    PatternMatchResult result = match(pattern, field, false, std::move(scoreCallback));
    if (!result.matched)
        return ComplementResult(false);

    if (result.unusedVariables.size() > numAllowingFillingUnusedVariables)
        return ComplementResult(false);

    bool ok = fillUnusedVariableColors(pattern, field, !result.unusedVariables.isEmpty(), result.unusedVariables, cpl);
    int filled = std::min(static_cast<int>(result.unusedVariables.size()),
                          numAllowingFillingUnusedVariables);
    return ComplementResult(ok, filled);
}

#endif
