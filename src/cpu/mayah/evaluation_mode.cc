#include "evaluation_mode.h"

#include <glog/logging.h>

using namespace std;

string toString(EvaluationMode mode)
{
    switch (mode) {
    case EvaluationMode::INITIAL: return "initial";
    case EvaluationMode::EARLY: return "early";
    case EvaluationMode::MIDDLE: return "middle";
    case EvaluationMode::LATE: return "late";
    case EvaluationMode::ENEMY_HAS_ZENKESHI: return "enemy_has_zenkeshi";
    case EvaluationMode::ENEMY_HAS_ZENKESHI_MIDDLE: return "enemy_has_zenkeshi_middle";
    default:
        CHECK(false) << "Unknown EvaluationMode: " << static_cast<int>(mode);
    }
}
