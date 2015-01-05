#ifndef CPU_MAYAH_PATTERN_FIELD_H_
#define CPU_MAYAH_PATTERN_FIELD_H_

#include <cstdint>
#include <string>
#include <vector>

#include "core/field_constant.h"

enum class PatternType : std::uint8_t {
    NONE, ANY, MUST_EMPTY, VAR, MUST_VAR, ALLOW_VAR, NOT_VAR,
};

// PatternField is a field that holds characters.
// This would be useful if you want to write an algorithm to do pattern-match.
//
// Generally we use 'A' to 'Z' for representing variables.
// ' ' or '.'  represents empty.
class PatternField : FieldConstant {
public:
    explicit PatternField(double defaultScore = 1);
    explicit PatternField(const std::string&, double defaultScore = 1);
    explicit PatternField(const std::vector<std::string>&, double defaultScore = 1);

    static bool merge(const PatternField&, const PatternField&, PatternField*);

    void setPattern(int x, int y, PatternType t, char variable, double score);

    int numVariables() const { return numVariables_; }

    int height(int x) const { return heights_[x]; }
    char variable(int x, int y) const { return vars_[x][y]; }
    double score(int x, int y) const { return scores_[x][y]; }
    PatternType type(int x, int y) const { return types_[x][y]; }

    PatternField mirror() const;

    std::string toDebugString() const;

    void setScore(int x, int y, double d) { scores_[x][y] = d; }
    void setType(int x, int y, PatternType t) { types_[x][y] = t; }
    void setVariable(int x, int y, char c) { vars_[x][y] = c; }

private:
    static PatternType inferType(char c, PatternType typeForLowerCase = PatternType::ALLOW_VAR);
    int countVariables() const;

    char vars_[MAP_WIDTH][MAP_HEIGHT];
    double scores_[MAP_WIDTH][MAP_HEIGHT];
    PatternType types_[MAP_WIDTH][MAP_HEIGHT];
    std::int8_t heights_[MAP_WIDTH];
    int numVariables_;
};

#endif // CPU_MAYAH_PATTERN_FIELD_H_
