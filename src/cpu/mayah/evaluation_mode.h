#ifndef CPU_MAYAH_EVALUATION_MODE_H_
#define CPU_MAYAH_EVALUATION_MODE_H_

#include <string>

#include "base/base.h"

enum class EvaluationMode {
    INITIAL,
    EARLY,
    MIDDLE,
    LATE,
    ENEMY_HAS_ZENKESHI,
    ENEMY_HAS_ZENKESHI_MIDDLE,
};

const EvaluationMode ALL_EVALUATION_MODES[] = {
    EvaluationMode::INITIAL,
    EvaluationMode::EARLY,
    EvaluationMode::MIDDLE,
    EvaluationMode::LATE,
    EvaluationMode::ENEMY_HAS_ZENKESHI,
    EvaluationMode::ENEMY_HAS_ZENKESHI_MIDDLE,
};

inline int ordinal(EvaluationMode mode) { return static_cast<int>(mode); }
std::string toString(EvaluationMode);
const int NUM_EVALUATION_MODES = ARRAY_SIZE(ALL_EVALUATION_MODES);

#endif // CPU_MAYAH_EVALUATION_MODE_H_
