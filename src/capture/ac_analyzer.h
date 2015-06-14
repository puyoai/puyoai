#ifndef CAPTURE_AC_ANALYZER_H_
#define CAPTURE_AC_ANALYZER_H_

#include "base/base.h"
#include "capture/analyzer.h"
#include "gui/bounding_box.h"  // TODO(mayah): Consider removing this
#include "recognition/recognizer.h"

struct Box;
struct HSV;
struct RGB;

class ACAnalyzer : public Analyzer {
public:
    ACAnalyzer();
    virtual ~ACAnalyzer();

    enum class AllowOjama { DONT_ALLOW_OJAMA, ALLOW_OJAMA };
    enum class ShowDebugMessage { DONT_SHOW_DEBUG, SHOW_DEBUG_MESSAGE };

    RealColor analyzeBox(const SDL_Surface*,
                         const Box&,
                         AllowOjama = AllowOjama::ALLOW_OJAMA,
                         ShowDebugMessage = ShowDebugMessage::DONT_SHOW_DEBUG) const;

    RealColor analyzeBoxWithRecognizer(const SDL_Surface*, const Box&) const;

    // Draw each pixel of |surface| with RealColor. This is helpful for image analyzing test.
    void drawWithAnalysisResult(SDL_Surface*);

    static RealColor estimateRealColor(const HSV&);

private:
    CaptureGameState detectGameState(const SDL_Surface*) override;
    std::unique_ptr<DetectedField> detectField(int pi,
                                               const SDL_Surface* current,
                                               const SDL_Surface* prev2) override;
    bool detectOjamaDrop(const SDL_Surface* current, const SDL_Surface* prev, const Box&);

    bool isLevelSelect(const SDL_Surface*);
    bool isGameFinished(const SDL_Surface*);

    bool isMatchEnd(const SDL_Surface*);
    bool isDead(int playerId, const SDL_Surface*);

    void drawBoxWithAnalysisResult(SDL_Surface*, const Box&);

    Recognizer recognizer_;
};

#endif
