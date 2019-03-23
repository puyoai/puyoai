#ifndef CAPTURE_ES_ANALYZER_H_
#define CAPTURE_ES_ANALYZER_H_

#include "capture/analyzer.h"

class ESAnalyzer : public Analyzer {
public:
    ESAnalyzer();
    virtual ~ESAnalyzer();

    // Draw each pixel of |surface| with RealColor. This is helpful for image analyzing test.
    void drawWithAnalysisResult(SDL_Surface*);

protected:
    // These methods should be implemented in the derived class.
    CaptureGameState detectGameState(const SDL_Surface*) override;
    std::unique_ptr<DetectedField> detectField(int pi,
                                               const SDL_Surface* current,
                                               const SDL_Surface* prev2,
                                               const SDL_Surface* prev3) override;
};

#endif
