#ifndef CORE_ALGORITHM_PATTERN_FIELD_H_
#define CORE_ALGORITHM_PATTERN_FIELD_H_

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "core/field_checker.h"
#include "core/field_constant.h"

class ColumnPuyoList;
class CoreField;
// class FieldChecker;
class PatternMatcher;
struct Position;

enum class PatternType {
    NONE, ANY, VAR, MUST_VAR, ALLOW_VAR,
    ALLOW_FILLING_IRON, WALL
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

    bool isMatchable(const CoreField&) const;

    void setPattern(int x, int y, PatternType t, char variable);

    int numVariables() const { return numVariables_; }
    double score() const { return score_; }

    int height(int x) const { return heights_[x]; }
    char variable(int x, int y) const { return vars_[x][y]; }
    PatternType type(int x, int y) const { return types_[x][y]; }

    Position* fillSameVariablePositions(int x, int y, char c, Position* positionQueueHead, FieldChecker*) const;

    FieldPattern mirror() const;

    std::string toDebugString() const;

    void setMustVar(int x, int y) { types_[x][y] = PatternType::MUST_VAR; } 

private:
    static PatternType inferType(char c);
    int countVariables() const;

    double score_;
    char vars_[MAP_WIDTH][MAP_HEIGHT];
    PatternType types_[MAP_WIDTH][MAP_HEIGHT];
    int heights_[MAP_WIDTH];
    int numVariables_;
};

#endif // CORE_ALGORITHM_PATTERN_FIELD_H_
