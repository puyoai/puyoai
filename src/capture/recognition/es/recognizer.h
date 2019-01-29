#ifndef CAPTURE_RECOGNITION_ES_ESRECOGNIZER_H_
#define CAPTURE_RECOGNITION_ES_ESRECOGNIZER_H_

#include <memory>

#include "capture/recognition/recognition_color.h"
#include "core/real_color.h"
#include "learning/arow.h"

class ESRecognizer {
public:
    ESRecognizer();

    RealColor recognize(const double features[40 * 43 * 3]) const;

private:
    std::unique_ptr<Arow> arows[NUM_RECOGNITION];
};

#endif // CAPTURE_RECOGNITION_RECOGNIZER_H_
