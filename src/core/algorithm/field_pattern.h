#ifndef CORE_ALGORITHM_PATTERN_FIELD_H_
#define CORE_ALGORITHM_PATTERN_FIELD_H_

#include <cstdint>
#include <string>
#include <vector>

#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/field_constant.h"
#include "core/field_bit_field.h"
#include "core/position.h"

class PatternMatcher;

enum class PatternType : std::uint8_t {
    NONE, ANY, MUST_EMPTY, VAR, MUST_VAR, ALLOW_VAR, NOT_VAR,
    ALLOW_FILLING_OJAMA, ALLOW_FILLING_IRON, WALL
};

struct ComplementResult {
    explicit ComplementResult(bool success, int filled = 0) :
        success(success), numFilledUnusedVariables(filled) {}

    bool success = false;
    int numFilledUnusedVariables = 0;
};

// FieldPattern is a field that holds characters.
// This would be useful if you want to write an algorithm to do pattern-match.
//
// Generally we use 'A' to 'Z' for representing variables.
// ' ' or '.'  represents empty.
class FieldPattern : FieldConstant {
public:
    explicit FieldPattern(double defaultScore = 1);
    explicit FieldPattern(const std::string&, double defaultScore = 1);
    explicit FieldPattern(const std::vector<std::string>&, double defaultScore = 1);

    static bool merge(const FieldPattern&, const FieldPattern&, FieldPattern*);

    bool isMatchable(const CoreField&) const;
    ComplementResult complement(const CoreField&,
                                int numAllowingFillingUnusedVariables,
                                ColumnPuyoList*) const;
    ComplementResult complement(const CoreField& cf, ColumnPuyoList* cpl) const
    {
        return complement(cf, 0, cpl);
    }

    void setPattern(int x, int y, PatternType t, char variable, double score);

    int numVariables() const { return numVariables_; }

    int height(int x) const { return heights_[x]; }
    char variable(int x, int y) const { return vars_[x][y]; }
    double score(int x, int y) const { return scores_[x][y]; }
    PatternType type(int x, int y) const { return types_[x][y]; }

    Position* fillSameVariablePositions(int x, int y, char c, Position* positionQueueHead, FieldBitField*) const;

    FieldPattern mirror() const;

    std::string toDebugString() const;

    void setScore(int x, int y, double d) { scores_[x][y] = d; }
    void setType(int x, int y, PatternType t) { types_[x][y] = t; }
    void setVariable(int x, int y, char c) { vars_[x][y] = c; }

private:
    static PatternType inferType(char c, PatternType typeForLowerCase = PatternType::ALLOW_VAR);
    int countVariables() const;

    bool fillUnusedVariableColors(const CoreField&,
                                  int pos,
                                  const std::vector<char>& unusedVariables,
                                  PatternMatcher*,
                                  ColumnPuyoList*) const;
    bool complementInternal(const CoreField&,
                            const PatternMatcher&,
                            ColumnPuyoList*) const;

    char vars_[MAP_WIDTH][MAP_HEIGHT];
    double scores_[MAP_WIDTH][MAP_HEIGHT];
    PatternType types_[MAP_WIDTH][MAP_HEIGHT];
    std::int8_t heights_[MAP_WIDTH];
    int numVariables_;
};

#endif // CORE_ALGORITHM_PATTERN_FIELD_H_
