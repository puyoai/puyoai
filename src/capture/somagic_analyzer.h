#ifndef CAPTURE_SOMAGIC_ANALYZER_H_
#define CAPTURE_SOMAGIC_ANALYZER_H_

#include "base/base.h"
#include "capture/analyzer.h"
#include "gui/bounding_box.h"  // TODO(mayah): Consider removing this

struct Box;
struct HSV;
struct RGB;

struct BoxAnalyzeResult {
    BoxAnalyzeResult(RealColor rc, bool vanishing)
    {
        this->realColor = rc;
        this->vanishing = vanishing;
    }

    RealColor realColor;
    bool vanishing;
};

class SomagicAnalyzer : public Analyzer {
public:
    SomagicAnalyzer();
    virtual ~SomagicAnalyzer();

    enum class AllowOjama { DONT_ALLOW_OJAMA, ALLOW_OJAMA };
    BoxAnalyzeResult analyzeBox(const SDL_Surface* surface, const Box& b, AllowOjama = AllowOjama::ALLOW_OJAMA, bool showsColor = false) const;

    // Draw each pixel of |surface| with RealColor. This is helpful for image analyzing test.
    void drawWithAnalysisResult(SDL_Surface*);

    static RealColor estimateRealColor(const HSV&);

private:
    virtual CaptureGameState detectGameState(const SDL_Surface*) override;
    virtual std::unique_ptr<DetectedField> detectField(int pi, const SDL_Surface* current, const SDL_Surface* prev) override;
    bool detectOjamaDrop(const SDL_Surface* current, const SDL_Surface* prev, const Box&);

    bool isLevelSelect(const SDL_Surface*);
    bool isGameFinished(const SDL_Surface*);

    void drawBoxWithAnalysisResult(SDL_Surface*, const Box&);
};

#endif
