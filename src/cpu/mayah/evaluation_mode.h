#ifndef CPU_MAYAH_EVALUATION_MODE_H_
#define CPU_MAYAH_EVALUATION_MODE_H_

#include <string>

#include "base/base.h"

enum class EvaluationMode {
    DEFAULT,
    INITIAL,
    EARLY,
    MIDDLE,
    LATE,
    ENEMY_HAS_ZENKESHI,
};

const EvaluationMode ALL_EVALUATION_MODES[] = {
    EvaluationMode::DEFAULT,
    EvaluationMode::INITIAL,
    EvaluationMode::EARLY,
    EvaluationMode::MIDDLE,
    EvaluationMode::LATE,
    EvaluationMode::ENEMY_HAS_ZENKESHI,
};
inline int ordinal(EvaluationMode mode) { return static_cast<int>(mode); }
std::string toString(EvaluationMode);
const int NUM_EVALUATION_MODES = ARRAY_SIZE(ALL_EVALUATION_MODES);

#endif
