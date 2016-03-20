#ifndef CAPTURE_RECOGNITION_RECOGNITION_COLOR_H_
#define CAPTURE_RECOGNITION_RECOGNITION_COLOR_H_

#include "core/real_color.h"

enum class RecognitionColor {
    RED,
    BLUE,
    YELLOW,
    GREEN,
    PURPLE,
    EMPTY,
    OJAMA,
    ZENKESHI
};
const int NUM_RECOGNITION = 8;

RealColor toRealColor(RecognitionColor);

#endif // CAPTURE_RECOGNITION_RECOGNITION_COLOR_H_
