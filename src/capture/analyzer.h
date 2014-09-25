#ifndef CAPTURE_ANALYZER_H_
#define CAPTURE_ANALYZER_H_

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SDL.h>

#include "core/field/field_bit_field.h"
#include "core/kumipuyo.h"
#include "core/next_puyo.h"
#include "core/real_color.h"
#include "core/state.h"
#include "gui/bounding_box.h"
#include "capture/real_color_field.h"

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

    void setRealColor(int x, int y, RealColor rc) { field.set(x, y, rc); }
    void setRealColor(NextPuyoPosition npp, RealColor rc) { nextPuyos[static_cast<int>(npp)] = rc; }
    RealColor realColor(int x, int y) const { return field.get(x, y); }
    RealColor realColor(NextPuyoPosition npp) const { return nextPuyos[static_cast<int>(npp)]; }

    bool isVanishing(int x, int y) const { return vanishing.get(x, y); }
    void setVanishing(int x, int y, bool flag) { vanishing.set(x, y, flag); }

    void setOjamaDropDetected(bool flag) { ojamaDropDetected = flag; }

    RealColorField field;
    FieldBitField vanishing;
    RealColor nextPuyos[NUM_NEXT_PUYO_POSITION];
    bool ojamaDropDetected = false;
    bool next1AxisMoving = false;
};

// AdjustedField is adjusted using the previous frames etc.
struct AdjustedField {
    AdjustedField();

    void setRealColor(int x, int y, RealColor rc) { field.set(x, y, rc); }
    void setRealColor(NextPuyoPosition npp, RealColor rc) { nextPuyos[static_cast<int>(npp)] = rc; }
    RealColor realColor(int x, int y) const { return field.get(x, y); }
    RealColor realColor(NextPuyoPosition npp) const { return nextPuyos[static_cast<int>(npp)]; }

    bool isVanishing(int x, int y) const { return vanishing.get(x, y); }
    void setVanishing(int x, int y, bool flag) { vanishing.set(x, y, flag); }

    RealColorField field;
    FieldBitField vanishing;
    RealColor nextPuyos[NUM_NEXT_PUYO_POSITION];
};

struct PlayerAnalyzerResult {
    void clear() { *this = PlayerAnalyzerResult(); }

    void copyRealColorFrom(NextPuyoPosition npp, const PlayerAnalyzerResult& par)
    {
        adjustedField.setRealColor(npp, par.adjustedField.realColor(npp));
    }

    bool next1IsValid() const
    {
        return isNormalColor(adjustedField.realColor(NextPuyoPosition::NEXT1_AXIS)) &&
            isNormalColor(adjustedField.realColor(NextPuyoPosition::NEXT1_CHILD));
    }

    bool next2IsValid() const
    {
        return isNormalColor(adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS)) &&
            isNormalColor(adjustedField.realColor(NextPuyoPosition::NEXT2_CHILD));
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

    int countVanishing(const RealColorField&, const FieldBitField& vanishing);
};

#endif
