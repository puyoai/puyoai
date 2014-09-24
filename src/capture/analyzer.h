#ifndef CAPTURE_ANALYZER_H_
#define CAPTURE_ANALYZER_H_

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SDL.h>

#include "core/kumipuyo.h"
#include "core/next_puyo.h"
#include "core/real_color.h"
#include "core/state.h"
#include "gui/bounding_box.h"

// TODO(mayah): Should be renamed?
enum class CaptureGameState {
    UNKNOWN,
    LEVEL_SELECT,  // Level select
    PLAYING,       //
    FINISHED,      // A game is finished.
};
std::string toString(CaptureGameState);

enum class NextPuyoState {
    STABLE,
    NEXT2_WILL_DISAPPEAR,
    NEXT2_WILL_APPEAR,
};
std::string toString(NextPuyoState);

// DetectedField contains the detected field as-is.
struct DetectedField {
    DetectedField();

    void setRealColor(int x, int y, RealColor prc) { puyos[x - 1][y - 1] = prc; }
    void setRealColor(NextPuyoPosition npp, RealColor prc) { nextPuyos[static_cast<int>(npp)] = prc; }
    RealColor realColor(int x, int y) const { return puyos[x - 1][y - 1]; }
    RealColor realColor(NextPuyoPosition npp) const { return nextPuyos[static_cast<int>(npp)]; }

    bool isVanishing(int x, int y) const { return vanishing[x-1][y-1]; }

    void setOjamaDropDetected(bool flag) { ojamaDropDetected = flag; }

    RealColor puyos[6][12];
    bool vanishing[6][12];
    RealColor nextPuyos[NUM_NEXT_PUYO_POSITION];
    bool ojamaDropDetected = false;
    bool next1AxisMoving = false;
};

// AdjustedField is adjusted using the previous frames etc.
struct AdjustedField {
    AdjustedField();

    RealColor puyos[6][12];
    bool vanishing[6][12];
    RealColor nextPuyos[NUM_NEXT_PUYO_POSITION];
};

struct PlayerAnalyzerResult {
    void setRealColor(int x, int y, RealColor prc) { adjustedField.puyos[x-1][y-1] = prc; }
    void setRealColor(NextPuyoPosition npp, RealColor prc) { adjustedField.nextPuyos[static_cast<int>(npp)] = prc; }
    RealColor realColor(int x, int y) const { return adjustedField.puyos[x-1][y-1]; }
    RealColor realColor(NextPuyoPosition npp) const { return adjustedField.nextPuyos[static_cast<int>(npp)]; }
    void setVanishing(int x, int y, bool flag) { adjustedField.vanishing[x-1][y-1] = flag; }

    void clear() {
        *this = PlayerAnalyzerResult();
    }

    void copyRealColorFrom(NextPuyoPosition npp, const PlayerAnalyzerResult& par) { setRealColor(npp, par.realColor(npp)); }

    bool next1IsValid() const
    {
        return isNormalColor(realColor(NextPuyoPosition::NEXT1_AXIS)) &&
            isNormalColor(realColor(NextPuyoPosition::NEXT1_CHILD));
    }

    bool next2IsValid() const
    {
        return isNormalColor(realColor(NextPuyoPosition::NEXT2_AXIS)) &&
            isNormalColor(realColor(NextPuyoPosition::NEXT2_CHILD));
    }

    // |state| should be true when:
    //   - the previous result does not exist
    //   - for test
    // Otherwise, |state| will be false.
    void resetCurrentPuyoState(bool state);

    std::string toString() const;

public: // Make this private?
    UserState userState;
    DetectedField detectedField;
    AdjustedField adjustedField;

    int numOjama = 0;
    int restFramesUserCanPlay = 0;

    NextPuyoState nextPuyoState = NextPuyoState::STABLE;
    int framesWhileNext1Disappearing = 0;
    int framesWhileNext2Disappearing = 0;
    std::map<std::pair<RealColor, RealColor>, int> next1Puyos;
    std::map<std::pair<RealColor, RealColor>, int> next2Puyos;

    // Make these false when a current puyo has appeared.
    bool hasDetectedRensaStart_ = true;
    bool hasSentGrounded_ = true;
    bool hasSentOjamaDropped_ = true;
    bool hasSentChainFinished_ = true;
    bool nextWillDisappearFast_ = false;
};

class AnalyzerResult {
public:
    AnalyzerResult(CaptureGameState, std::unique_ptr<PlayerAnalyzerResult> p1, std::unique_ptr<PlayerAnalyzerResult> p2);
    ~AnalyzerResult() {}

    CaptureGameState state() const { return gameState_; }
    const PlayerAnalyzerResult* playerResult(int pi) const { return playerResults_[pi].get(); }
    PlayerAnalyzerResult* mutablePlayerResult(int pi) { return playerResults_[pi].get(); }

    std::unique_ptr<AnalyzerResult> copy() const;

    void clear()
    {
        if (playerResults_[0].get())
            playerResults_[0]->clear();
        if (playerResults_[1].get())
            playerResults_[1]->clear();
    }

    std::string toString() const;

private:
    CaptureGameState gameState_;
    std::unique_ptr<PlayerAnalyzerResult> playerResults_[2];
};

class Analyzer {
public:
    virtual ~Analyzer() {}

    // Analyzes the specified frame. previousResults.front() should be the most recent results.
    std::unique_ptr<AnalyzerResult> analyze(const SDL_Surface* current,
                                            const SDL_Surface* prev,
                                            const std::deque<std::unique_ptr<AnalyzerResult>>& previousResults);

protected:
    // These methods should be implemented in the derived class.
    virtual CaptureGameState detectGameState(const SDL_Surface*) = 0;
    virtual std::unique_ptr<DetectedField> detectField(int pi, const SDL_Surface* current, const SDL_Surface* prev) = 0;

private:
    std::unique_ptr<PlayerAnalyzerResult>
    analyzePlayerField(const DetectedField&, const std::vector<const PlayerAnalyzerResult*>& previousResults);
    std::unique_ptr<PlayerAnalyzerResult>
    analyzePlayerFieldOnLevelSelect(const DetectedField&, const std::vector<const PlayerAnalyzerResult*>& previousResults);

    void analyzeNext(const DetectedField&,
                     const std::vector<const PlayerAnalyzerResult*>& previousResults,
                     PlayerAnalyzerResult*);

    void analyzeNextForLevelSelect(const DetectedField&, PlayerAnalyzerResult*);
    void analyzeNextWhenPreviousResultDoesNotExist(const DetectedField&, PlayerAnalyzerResult*);

    void analyzeNextForStateStable(const DetectedField&, PlayerAnalyzerResult*);
    void analyzeNextForStateNext2WillDisappear(const DetectedField&, PlayerAnalyzerResult*);
    void analyzeNextForStateNext2WillAppear(const DetectedField&, PlayerAnalyzerResult*);

    void analyzeField(const DetectedField&,
                      const std::vector<const PlayerAnalyzerResult*>& previousResults,
                      PlayerAnalyzerResult*);
    void analyzeFieldForLevelSelect(const DetectedField&, PlayerAnalyzerResult*);

    int countVanishing(RealColor puyos[6][12], bool vanishing[6][12]);
};

#endif
