#pragma once

#include <string>
#include <cstddef>
#include <limits>

namespace munetoshi {

const std::string AI_NAME("minim");

typedef int grade;
constexpr grade GRADE_MIN = std::numeric_limits<grade>::min();

enum class EVALUATOR_TYPES {
    // Plan evaluators
    DEATH_RATIO,
    TEAR,
    VALLEY_SHAPE,

    // Possible chain evaluators
    CHAIN_LENGTH,
    NUM_REQUIRED_PUYO,
    TURNOVER_SHAPE,

    _NIL_TYPE,
};

constexpr size_t NUM_EVALUATOR_TYPES = static_cast<size_t>(EVALUATOR_TYPES::_NIL_TYPE);;;;

} // namespace munetoshi
