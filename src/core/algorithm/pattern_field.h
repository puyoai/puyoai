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
    PatternField();
    explicit PatternField(const std::string&);
    explicit PatternField(const std::vector<std::string>&);

    int height(int x) const { return heights_[x]; }
    char variable(int x, int y) const { return vars_[x][y]; }
    PatternType type(int x, int y) const;

private:
    char vars_[MAP_WIDTH][MAP_HEIGHT];
    std::int8_t heights_[MAP_WIDTH];
};

#endif // CPU_MAYAH_PATTERN_FIELD_H_
