#ifndef CAPTURE_RECOGNITION_RECOGNIZER_H_
#define CAPTURE_RECOGNITION_RECOGNIZER_H_

#include "capture/recognition/arow.h"
#include "core/real_color.h"

class Recognizer {
public:
    Recognizer();

    RealColor recognize(const double features[16 * 16 * 3]) const;

private:
    Arow arows[NUM_RECOGNITION];
};

#endif // CAPTURE_RECOGNITION_RECOGNIZER_H_
