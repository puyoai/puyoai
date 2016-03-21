#include "capture/recognition/recognizer.h"

#include <algorithm>
#include <vector>

#include "capture/recognition/classifier_features.h"

using namespace std;

Recognizer::Recognizer()
{
    arows[static_cast<int>(RecognitionColor::RED)].setMean(std::vector<double>(RED_MEAN, RED_MEAN + RED_MEAN_SIZE));
    arows[static_cast<int>(RecognitionColor::BLUE)].setMean(std::vector<double>(BLUE_MEAN, BLUE_MEAN + BLUE_MEAN_SIZE));
    arows[static_cast<int>(RecognitionColor::YELLOW)].setMean(std::vector<double>(YELLOW_MEAN, YELLOW_MEAN + YELLOW_MEAN_SIZE));
    arows[static_cast<int>(RecognitionColor::GREEN)].setMean(std::vector<double>(GREEN_MEAN, GREEN_MEAN + GREEN_MEAN_SIZE));
    arows[static_cast<int>(RecognitionColor::PURPLE)].setMean(std::vector<double>(PURPLE_MEAN, PURPLE_MEAN + PURPLE_MEAN_SIZE));
    arows[static_cast<int>(RecognitionColor::EMPTY)].setMean(std::vector<double>(EMPTY_MEAN, EMPTY_MEAN + EMPTY_MEAN_SIZE));
    arows[static_cast<int>(RecognitionColor::OJAMA)].setMean(std::vector<double>(OJAMA_MEAN, OJAMA_MEAN + OJAMA_MEAN_SIZE));
    arows[static_cast<int>(RecognitionColor::ZENKESHI)].setMean(std::vector<double>(ZENKESHI_MEAN, ZENKESHI_MEAN + ZENKESHI_MEAN_SIZE));

    arows[static_cast<int>(RecognitionColor::RED)].setCov(std::vector<double>(RED_COV, RED_COV + RED_COV_SIZE));
    arows[static_cast<int>(RecognitionColor::BLUE)].setCov(std::vector<double>(BLUE_COV, BLUE_COV + BLUE_COV_SIZE));
    arows[static_cast<int>(RecognitionColor::YELLOW)].setCov(std::vector<double>(YELLOW_COV, YELLOW_COV + YELLOW_COV_SIZE));
    arows[static_cast<int>(RecognitionColor::GREEN)].setCov(std::vector<double>(GREEN_COV, GREEN_COV + GREEN_COV_SIZE));
    arows[static_cast<int>(RecognitionColor::PURPLE)].setCov(std::vector<double>(PURPLE_COV, PURPLE_COV + PURPLE_COV_SIZE));
    arows[static_cast<int>(RecognitionColor::EMPTY)].setCov(std::vector<double>(EMPTY_COV, EMPTY_COV + EMPTY_COV_SIZE));
    arows[static_cast<int>(RecognitionColor::OJAMA)].setCov(std::vector<double>(OJAMA_COV, OJAMA_COV + OJAMA_COV_SIZE));
    arows[static_cast<int>(RecognitionColor::ZENKESHI)].setCov(std::vector<double>(ZENKESHI_COV, ZENKESHI_COV + ZENKESHI_COV_SIZE));

}

RealColor Recognizer::recognize(const double features[16 * 16 * 3]) const
{
    double vs[NUM_RECOGNITION];
    for (int i = 0; i < NUM_RECOGNITION; ++i)
        vs[i] = arows[i].margin(features);

    int idx = std::max_element(vs, vs + NUM_RECOGNITION) - vs;
    return toRealColor(static_cast<RecognitionColor>(idx));
}
