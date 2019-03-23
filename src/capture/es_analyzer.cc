#include "capture/es_analyzer.h"

ESAnalyzer::ESAnalyzer() {}

ESAnalyzer::~ESAnalyzer() {}

void ESAnalyzer::drawWithAnalysisResult(SDL_Surface*) {
}

CaptureGameState ESAnalyzer::detectGameState(const SDL_Surface* /*surface*/)
{
    return CaptureGameState::PLAYING;
}

std::unique_ptr<DetectedField> ESAnalyzer::detectField(int /*player_index*/,
                                                       const SDL_Surface* /*surface0*/,
                                                       const SDL_Surface* /*surface1*/,
                                                       const SDL_Surface* /*surface2*/)
{
    std::unique_ptr<DetectedField> result(new DetectedField);
    return result;
}
