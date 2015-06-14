#ifndef RECOGNITION_RECOGNIZER_H_
#define RECOGNITION_RECOGNIZER_H_

#include "core/real_color.h"
#include "recognition/arow.h"

class Recognizer {
public:
    explicit Recognizer(const std::string& dir);

    RealColor recognize(const double features[16 * 16 * 3]) const;

private:
    Arow arows[NUM_RECOGNITION];
};

#endif // RECOGNITION_RECOGNIZER_H_
