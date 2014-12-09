#ifndef CPU_MAYAH_PATTERN_FIELD_H_
#define CPU_MAYAH_PATTERN_FIELD_H_

#include <cstdint>
#include <string>
#include <vector>

#include "core/field_constant.h"

// PatternField is a field that holds characters.
// This would be useful if you want to write an algorithm to do pattern-match.
//
// Generally we use 'A' to 'Z' for representing variables.
// ' ' or '.'  represents empty.
class PatternField : FieldConstant {
public:
    PatternField();
    explicit PatternField(const std::vector<std::string>&);

    int height(int x) const { return heights_[x]; }
    char get(int x, int y) const { return field_[x][y]; }

private:
    std::int8_t heights_[MAP_WIDTH];
    char field_[MAP_WIDTH][MAP_HEIGHT];
};

#endif // CPU_MAYAH_PATTERN_FIELD_H_
