#pragma once

#include<cstddef>
#include<limits>

namespace munetoshi {

typedef float grade;
constexpr grade GRADE_MIN = std::numeric_limits<grade>::min();

enum class EVALUATOR_TYPES {
    CHAIN_LENGTH,
    NUM_REQUIRED_PUYO,
    DEATH_RATIO,
    TEAR,
    VALLEY_SHAPE,
    TURNOVER_SHAPE,

    _NIL_TYPE,
};

constexpr size_t NUM_EVALUATOR_TYPES = static_cast<size_t>(EVALUATOR_TYPES::_NIL_TYPE);;;;

} // namespace munetoshi
