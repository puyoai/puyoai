#include "recognition/recognizer.h"

#include <algorithm>

using namespace std;

Recognizer::Recognizer(const std::string& dir)
{
    arows[static_cast<int>(RecognitionColor::RED)].load(dir + "/red.arow");
    arows[static_cast<int>(RecognitionColor::BLUE)].load(dir + "/blue.arow");
    arows[static_cast<int>(RecognitionColor::YELLOW)].load(dir + "/yellow.arow");
    arows[static_cast<int>(RecognitionColor::GREEN)].load(dir + "/green.arow");
    arows[static_cast<int>(RecognitionColor::PURPLE)].load(dir + "/purple.arow");
    arows[static_cast<int>(RecognitionColor::EMPTY)].load(dir + "/empty.arow");
    arows[static_cast<int>(RecognitionColor::OJAMA)].load(dir + "/ojama.arow");
    arows[static_cast<int>(RecognitionColor::ZENKESHI)].load(dir + "/zenkeshi.arow");
}

RealColor Recognizer::recognize(const double features[16 * 16 * 3]) const
{
    double vs[NUM_RECOGNITION];
    for (int i = 0; i < NUM_RECOGNITION; ++i)
        vs[i] = arows[i].margin(features);

    int idx = std::max_element(vs, vs + NUM_RECOGNITION) - vs;
    return toRealColor(static_cast<RecognitionColor>(idx));
}
