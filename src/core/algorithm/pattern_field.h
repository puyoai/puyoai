#ifndef CPU_MAYAH_PATTERN_FIELD_H_
#define CPU_MAYAH_PATTERN_FIELD_H_

#include <cstdint>
#include <string>
#include <vector>

#include "core/field_constant.h"

enum class PatternType : std::uint8_t {
    EMPTY, ANY, MUST, ALLOW_VAR, MUST_VAR,
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

    int numVariables() const { return numVariables_; }

    int height(int x) const { return heights_[x]; }
    char variable(int x, int y) const { return vars_[x][y]; }
    double score(int x, int y) const { return scores_[x][y]; }
    PatternType type(int x, int y) const;

    PatternField mirror() const;

    std::string toDebugString() const;

    void setScore(int x, int y, double d) { scores_[x][y] = d; }
    void setVariable(int x, int y, char c) { vars_[x][y] = c; }
    bool merge(const PatternField&);

private:
    int countVariables() const;

    char vars_[MAP_WIDTH][MAP_HEIGHT];
    double scores_[MAP_WIDTH][MAP_HEIGHT];
    std::int8_t heights_[MAP_WIDTH];
    int numVariables_;
};

#endif // CPU_MAYAH_PATTERN_FIELD_H_
