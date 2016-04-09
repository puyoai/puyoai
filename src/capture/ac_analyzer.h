#ifndef CAPTURE_AC_ANALYZER_H_
#define CAPTURE_AC_ANALYZER_H_

#include "base/base.h"
#include "capture/analyzer.h"
#include "capture/recognition/recognizer.h"
#include "gui/bounding_box.h"  // TODO(mayah): Consider removing this

struct Box;
struct HSV;
struct RGB;

class ACAnalyzer : public Analyzer {
public:
    ACAnalyzer();
    virtual ~ACAnalyzer();

    enum class AllowOjama { DONT_ALLOW_OJAMA, ALLOW_OJAMA };
    enum class ShowDebugMessage { DONT_SHOW_DEBUG, SHOW_DEBUG_MESSAGE };
    enum class AnalyzeBoxFunc { NORMAL, NEXT2 };

    RealColor analyzeBox(const SDL_Surface*,
                         const Box&,
                         AllowOjama = AllowOjama::ALLOW_OJAMA,
                         ShowDebugMessage = ShowDebugMessage::DONT_SHOW_DEBUG,
                         AnalyzeBoxFunc = AnalyzeBoxFunc::NORMAL) const;

    RealColor analyzeBoxWithRecognizer(const SDL_Surface*, const Box&) const;

    RealColor analyzeBoxInField(const SDL_Surface*, const Box&) const;
    RealColor analyzeBoxNext2(const SDL_Surface*, const Box&) const;

    // Draw each pixel of |surface| with RealColor. This is helpful for image analyzing test.
    void drawWithAnalysisResult(SDL_Surface*);

    CaptureGameState detectGameState(const SDL_Surface*) override;

    // For testing.
    static RealColor estimatePixelRealColor(const RGB&);

private:
    std::unique_ptr<DetectedField> detectField(int pi,
                                               const SDL_Surface* current,
                                               const SDL_Surface* prev2,
                                               const SDL_Surface* prev3) override;
    bool detectOjamaDrop(const SDL_Surface* current, const SDL_Surface* prev, const Box&);

    bool isLevelSelect(const SDL_Surface*);
    bool isGameFinished(const SDL_Surface*);

    bool isMatchEnd(const SDL_Surface*);
    bool isDead(int playerId, const SDL_Surface*);

    void drawBoxWithAnalysisResult(SDL_Surface*, const Box&);

    Recognizer recognizer_;
};

#endif
