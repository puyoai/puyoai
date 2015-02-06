#include "capture/analyzer.h"

#include <queue>

#include "core/constant.h"

using namespace std;

namespace {

// TODO(mayah): Should we make this 3, since we've seen puyo miscontrolling?
// However, even 3, I've seen puyo miscontrolling.
const int NUM_FRAMES_TO_MOVE_AFTER_NEXT1_DISAPPEARING = 2;

const int NUM_FRAMES_TO_SEE_FOR_FIELD = 5;
const int NUM_FRAMES_BEFORE_USER_CAN_PLAY = 2;

// The pointer will be alive while previousResults are alive.
vector<const PlayerAnalyzerResult*> makePlayerOnlyResults(int pi, const deque<unique_ptr<AnalyzerResult>>& previousResults)
{
    vector<const PlayerAnalyzerResult*> vs;
    vs.reserve(previousResults.size());
    for (const auto& ar : previousResults) {
        const PlayerAnalyzerResult* p = ar->playerResult(pi);
        if (p)
            vs.push_back(p);
    }

    return vs;
}
}

string toString(CaptureGameState cgs)
{
    switch (cgs) {
    case CaptureGameState::UNKNOWN:      return "unknown";
    case CaptureGameState::LEVEL_SELECT: return "level select";
    case CaptureGameState::PLAYING:      return "playing";
    case CaptureGameState::FINISHED:     return "finished";
    }

    CHECK(false) << "Unknown CaptureGameState: "  << static_cast<int>(cgs);
    return string();
}

string toString(NextPuyoState nps)
{
    switch (nps) {
    case NextPuyoState::STABLE:               return "stable";
    case NextPuyoState::NEXT2_WILL_DISAPPEAR: return "next2_will_disappear";
    case NextPuyoState::NEXT2_WILL_APPEAR:    return "next2_will_appear";
    }

    CHECK(false) << "Unknown NextPuyoState: " << static_cast<int>(nps);
    return string();
}

DetectedField::DetectedField()
{
    for (int i = 0; i < NUM_NEXT_PUYO_POSITION; ++i) {
        nextPuyos[i] = RealColor::RC_EMPTY;
    }
}

AdjustedField::AdjustedField()
{
    for (int i = 0; i < NUM_NEXT_PUYO_POSITION; ++i) {
        nextPuyos[i] = RealColor::RC_EMPTY;
    }
}

void PlayerAnalyzerResult::resetCurrentPuyoState(bool state)
{
    nextWillDisappearFast_ = false;
    nextHasDisappearedIrregularly_ = false;

    hasDetectedRensaStart_ = state;
    hasDetectedPuyoErase_ = state;
    hasSentGrounded_ = state;
    hasSentOjamaDropped_ = state;
    hasSentChainFinished_ = state;
}

string PlayerAnalyzerResult::toString() const
{
    stringstream ss;

    ss << "nextPuyoState: " << static_cast<int>(nextPuyoState) << endl;
    ss << "userEvent" << userEvent.toString() << endl;

    ss << "Detected Field    Adjusted Field" << endl;
    ss << "########          ########" << endl;
    for (int y = 12; y >= 1; --y) {
        ss << '#';
        for (int x = 1; x <= 6; ++x) {
            ss << toChar(detectedField.field.get(x, y), !detectedField.vanishing.get(x, y));
        }
        ss << "#          #";
        for (int x = 1; x <= 6; ++x) {
            ss << toChar(adjustedField.field.get(x, y), !adjustedField.vanishing.get(x, y));
        }
        ss << '#';
        ss << endl;
    }
    ss << "########          ########" << endl;

    return ss.str();
}

AnalyzerResult::AnalyzerResult(CaptureGameState cgs,
                               unique_ptr<PlayerAnalyzerResult> p1,
                               unique_ptr<PlayerAnalyzerResult> p2) :
    gameState_(cgs),
    playerResults_{move(p1), move(p2)}
{
}

string AnalyzerResult::toString() const
{
    ostringstream oss;
    oss << "State: " << ::toString(gameState_) << endl;

    for (int pi = 0; pi < 2; ++pi) {
        const PlayerAnalyzerResult* par = playerResult(pi);
        if (!par)
            continue;

        oss << "Player " << pi << ": " << par->toString() << endl;
    }

    return oss.str();
}

unique_ptr<AnalyzerResult> AnalyzerResult::copy() const
{
    unique_ptr<PlayerAnalyzerResult> p1(playerResult(0) ? new PlayerAnalyzerResult(*playerResult(0)) : nullptr);
    unique_ptr<PlayerAnalyzerResult> p2(playerResult(1) ? new PlayerAnalyzerResult(*playerResult(1)) : nullptr);
    return unique_ptr<AnalyzerResult>(new AnalyzerResult(state(), move(p1), move(p2)));
}

std::unique_ptr<AnalyzerResult> Analyzer::analyze(const SDL_Surface* surface,
                                                  const SDL_Surface* prevSurface,
                                                  const deque<unique_ptr<AnalyzerResult>>& previousResults)
{
    auto gameState = detectGameState(surface);
    auto player1FieldResult = detectField(0, surface, prevSurface);
    auto player2FieldResult = detectField(1, surface, prevSurface);

    switch (gameState) {
    case CaptureGameState::UNKNOWN: {
        // When in unknown state, we don't check the player field.
        auto player1Result = unique_ptr<PlayerAnalyzerResult>();
        auto player2Result = unique_ptr<PlayerAnalyzerResult>();
        return std::unique_ptr<AnalyzerResult>(new AnalyzerResult(gameState, move(player1Result), move(player2Result)));
    }
    case CaptureGameState::LEVEL_SELECT: {
        auto player1Result = analyzePlayerFieldOnLevelSelect(*player1FieldResult, makePlayerOnlyResults(0, previousResults));
        auto player2Result = analyzePlayerFieldOnLevelSelect(*player2FieldResult, makePlayerOnlyResults(1, previousResults));
        return std::unique_ptr<AnalyzerResult>(new AnalyzerResult(gameState, move(player1Result), move(player2Result)));
    }
    case CaptureGameState::PLAYING: {
        auto player1Result = analyzePlayerField(*player1FieldResult, makePlayerOnlyResults(0, previousResults));
        auto player2Result = analyzePlayerField(*player2FieldResult, makePlayerOnlyResults(1, previousResults));
        return std::unique_ptr<AnalyzerResult>(new AnalyzerResult(gameState, move(player1Result), move(player2Result)));
    }
    case CaptureGameState::FINISHED: {
        // After finished, we don't need to check each player gamestate.
        auto player1Result = unique_ptr<PlayerAnalyzerResult>();
        auto player2Result = unique_ptr<PlayerAnalyzerResult>();
        return std::unique_ptr<AnalyzerResult>(new AnalyzerResult(gameState, move(player1Result), move(player2Result)));
    }
    }

    DCHECK(false) << "Unknown gamestate: " << static_cast<int>(gameState);
    return std::unique_ptr<AnalyzerResult>();
}

unique_ptr<PlayerAnalyzerResult>
Analyzer::analyzePlayerFieldOnLevelSelect(const DetectedField& detectedField, const vector<const PlayerAnalyzerResult*>& previousResults)
{
    // Note that Next should be analyzed before Field, since we use the result of the analysis.
    unique_ptr<PlayerAnalyzerResult> result(new PlayerAnalyzerResult);
    // Copy the previous state, however, clear event states.
    if (!previousResults.empty()) {
        *result = *previousResults.front();
        result->userEvent.clear();
    }

    // Note that Next should be analyzed before Field, since we use the result of the analysis.
    analyzeNextForLevelSelect(detectedField, result.get());
    analyzeFieldForLevelSelect(detectedField, result.get());

    result->resetCurrentPuyoState(true);
    result->nextWillDisappearFast_ = true;

    return result;
}

unique_ptr<PlayerAnalyzerResult>
Analyzer::analyzePlayerField(const DetectedField& detectedField, const vector<const PlayerAnalyzerResult*>& previousResults)
{
    unique_ptr<PlayerAnalyzerResult> result(new PlayerAnalyzerResult);
    if (!previousResults.empty()) {
        // Copy the previous state, however, clear event states.
        *result = *previousResults.front();
        result->userEvent.clear();
    } else {
        // When previous result does not exist, we reset the puyo state for testing.
        result->resetCurrentPuyoState(false);
    }

    // Note that Next should be analyzed before Field, since we use the result of the analysis.
    analyzeNext(detectedField, previousResults, result.get());
    analyzeField(detectedField, previousResults, result.get());

    // --- Finalize the current state
    if (result->restFramesUserCanPlay > 0) {
        result->restFramesUserCanPlay -= 1;
        if (result->restFramesUserCanPlay == 0 && !result->playable) {
            result->playable = true;
        }

        // Even if restFramesUserCanPlay_[pi] > 0, a puyo might appear on (3, 12).
        // In that case, we skip the rest of waiting frame.
        if (result->adjustedField.realColor(3, 12) != RealColor::RC_EMPTY) {
            result->restFramesUserCanPlay = 0;
            result->playable = true;
        }
    }

    return result;
}

void Analyzer::analyzeNext(const DetectedField& detectedField,
                           const vector<const PlayerAnalyzerResult*>& previousResults,
                           PlayerAnalyzerResult* result)
{
    // When the previous result does not exist, we will estimate from the current field.
    if (previousResults.empty()) {
        return analyzeNextWhenPreviousResultDoesNotExist(detectedField, result);
    }

    switch (previousResults.front()->nextPuyoState) {
    case NextPuyoState::STABLE:
        return analyzeNextForStateStable(detectedField, result);
    case NextPuyoState::NEXT2_WILL_DISAPPEAR:
        return analyzeNextForStateNext2WillDisappear(detectedField, result);
    case NextPuyoState::NEXT2_WILL_APPEAR:
        return analyzeNextForStateNext2WillAppear(detectedField, result);
    }
}

void Analyzer::analyzeNextForLevelSelect(const DetectedField& detectedField, PlayerAnalyzerResult* result)
{
    result->nextPuyoState = NextPuyoState::STABLE;
    result->playable = false;
    result->restFramesUserCanPlay = 0;
    result->resetCurrentPuyoState(true);
    result->nextWillDisappearFast_ = true;

    // There should not exist moving puyos. So, CURRENT_AXIS and CURRENT_CHILD should be empty.
    result->adjustedField.setRealColor(NextPuyoPosition::CURRENT_AXIS, RealColor::RC_EMPTY);
    result->adjustedField.setRealColor(NextPuyoPosition::CURRENT_CHILD, RealColor::RC_EMPTY);

    // NOTE: When axisColor or childColor is not normal, we skip this analysis.
    // This is because sometimes we miss-detect the next field.

    // Check NEXT1
    if (!result->next1IsValid()) {
        RealColor axisColor = detectedField.realColor(NextPuyoPosition::NEXT1_AXIS);
        RealColor childColor = detectedField.realColor(NextPuyoPosition::NEXT1_CHILD);

        if (isNormalColor(axisColor) && isNormalColor(childColor)) {
            int k = ++result->next1Puyos[make_pair(axisColor, childColor)];
            if (k >= 3) {
                result->adjustedField.setRealColor(NextPuyoPosition::NEXT1_AXIS, axisColor);
                result->adjustedField.setRealColor(NextPuyoPosition::NEXT1_CHILD, childColor);
                result->next1Puyos.clear();
            }
        }
    }

    // Check NEXT2
    if (!result->next2IsValid()) {
        RealColor axisColor = detectedField.realColor(NextPuyoPosition::NEXT2_AXIS);
        RealColor childColor = detectedField.realColor(NextPuyoPosition::NEXT2_CHILD);

        if (isNormalColor(axisColor) && isNormalColor(childColor)) {
            int k = ++result->next2Puyos[make_pair(axisColor, childColor)];
            if (k >= 3) {
                result->adjustedField.setRealColor(NextPuyoPosition::NEXT2_AXIS, axisColor);
                result->adjustedField.setRealColor(NextPuyoPosition::NEXT2_CHILD, childColor);
                // TODO(mayah): Need to check NEXT1 has been found?
                result->userEvent.wnextAppeared = true;
                result->next2Puyos.clear();
            }
        }
    }
}

void Analyzer::analyzeNextWhenPreviousResultDoesNotExist(const DetectedField& detectedField, PlayerAnalyzerResult* result)
{
    // We cannot do much thing. Let's consider that the current next state is STABLE.
    result->nextPuyoState = NextPuyoState::STABLE;
    result->playable = false;
    result->restFramesUserCanPlay = 0;
    result->resetCurrentPuyoState(false);
    result->nextWillDisappearFast_ = false;

    // We cannot detect moving puyos correctly. So, make them empty.
    result->adjustedField.setRealColor(NextPuyoPosition::CURRENT_AXIS, RealColor::RC_EMPTY);
    result->adjustedField.setRealColor(NextPuyoPosition::CURRENT_CHILD, RealColor::RC_EMPTY);

    // Just copy the detected field.
    result->adjustedField.setRealColor(NextPuyoPosition::NEXT1_AXIS,
                                       detectedField.realColor(NextPuyoPosition::NEXT1_AXIS));
    result->adjustedField.setRealColor(NextPuyoPosition::NEXT1_CHILD,
                                       detectedField.realColor(NextPuyoPosition::NEXT1_CHILD));
    result->adjustedField.setRealColor(NextPuyoPosition::NEXT2_AXIS,
                                       detectedField.realColor(NextPuyoPosition::NEXT2_AXIS));
    result->adjustedField.setRealColor(NextPuyoPosition::NEXT2_CHILD,
                                       detectedField.realColor(NextPuyoPosition::NEXT2_CHILD));
}

void Analyzer::analyzeNextForStateStable(const DetectedField& detectedField, PlayerAnalyzerResult* result)
{
    // After ojama drop or first hand, CURRENT puyo will appear just in 3 frames
    // after NEXT1 has moved. In other cases, NEXT1 will appear in 5 or 6 frames.

    // Check whether NEXT1 disappears.
    if (result->nextWillDisappearFast_ && detectedField.next1AxisMoving) {
        // In this case, we think NEXT1 has disappeared.
        // Otherwise, it will take a few frames to move puyo correctly.
        result->framesWhileNext1Disappearing = NUM_FRAMES_TO_MOVE_AFTER_NEXT1_DISAPPEARING;
    } else {
        RealColor axisColor = detectedField.realColor(NextPuyoPosition::NEXT1_AXIS);
        RealColor childColor = detectedField.realColor(NextPuyoPosition::NEXT1_CHILD);

        // NEXT1 has not disappeared yet.
        if (axisColor != RealColor::RC_EMPTY && childColor != RealColor::RC_EMPTY) {
            result->framesWhileNext1Disappearing = 0;
            return;
        }
    }

    // Detected NEXT1 has disappeard.
    result->framesWhileNext1Disappearing += 1;

    // When NEXT2 is not stabilized, we cannot proceed the current state.
    RealColor next2AxisColor = result->adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS);
    RealColor next2ChildColor = result->adjustedField.realColor(NextPuyoPosition::NEXT2_CHILD);
    if (!isNormalColor(next2AxisColor) || !isNormalColor(next2ChildColor))
        return;

    // We want to see NEXT1 is absent in <NUM_FRAMES_TO_MOVE_AFTER_NEXT1_DISAPPEARING> frames for stability.
    if (result->framesWhileNext1Disappearing < NUM_FRAMES_TO_MOVE_AFTER_NEXT1_DISAPPEARING)
        return;

    // Detected Next1 disappeared
    result->restFramesUserCanPlay = NUM_FRAMES_BEFORE_USER_CAN_PLAY;
    result->nextPuyoState = NextPuyoState::NEXT2_WILL_DISAPPEAR;
    result->playable = false;
    result->userEvent.decisionRequest = true;
    result->adjustedField.setRealColor(NextPuyoPosition::CURRENT_AXIS, result->adjustedField.realColor(NextPuyoPosition::NEXT1_AXIS));
    result->adjustedField.setRealColor(NextPuyoPosition::CURRENT_CHILD, result->adjustedField.realColor(NextPuyoPosition::NEXT1_CHILD));
    result->adjustedField.setRealColor(NextPuyoPosition::NEXT1_AXIS, result->adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS));
    result->adjustedField.setRealColor(NextPuyoPosition::NEXT1_CHILD, result->adjustedField.realColor(NextPuyoPosition::NEXT2_CHILD));
    result->adjustedField.setRealColor(NextPuyoPosition::NEXT2_AXIS, RealColor::RC_EMPTY);
    result->adjustedField.setRealColor(NextPuyoPosition::NEXT2_CHILD, RealColor::RC_EMPTY);

    if (result->hasDetectedRensaStart_ && !result->hasSentChainFinished_) {
        result->userEvent.chainFinished = true;
        result->hasSentChainFinished_ = true;
    }
}

void Analyzer::analyzeNextForStateNext2WillDisappear(const DetectedField& detectedField, PlayerAnalyzerResult* result)
{
    // Stay until next2 disappears.
    RealColor axisColor = detectedField.realColor(NextPuyoPosition::NEXT2_AXIS);
    RealColor childColor = detectedField.realColor(NextPuyoPosition::NEXT2_CHILD);

    if (axisColor == RealColor::RC_EMPTY || childColor == RealColor::RC_EMPTY) {
        result->framesWhileNext2Disappearing += 1;
        if (result->framesWhileNext2Disappearing >= 2) {
            result->nextPuyoState = NextPuyoState::NEXT2_WILL_APPEAR;
            result->framesWhileNext2Disappearing = 0;
            return;
        }
    } else {
        result->framesWhileNext2Disappearing = 0;
    }
}

void Analyzer::analyzeNextForStateNext2WillAppear(const DetectedField& detectedField, PlayerAnalyzerResult* result)
{
    // Stay until next2 appears.
    RealColor axisColor = detectedField.realColor(NextPuyoPosition::NEXT2_AXIS);
    RealColor childColor = detectedField.realColor(NextPuyoPosition::NEXT2_CHILD);

    if (isNormalColor(axisColor) && isNormalColor(childColor)) {
        pair<RealColor, RealColor> kp(axisColor, childColor);
        result->next2Puyos[kp]++;
        if (result->next2Puyos[kp] >= 3) {
            result->adjustedField.setRealColor(NextPuyoPosition::NEXT2_AXIS, axisColor);
            result->adjustedField.setRealColor(NextPuyoPosition::NEXT2_CHILD, childColor);
            result->nextPuyoState = NextPuyoState::STABLE;
            result->userEvent.wnextAppeared = true;
            result->next2Puyos.clear();
        }
    }
}

void Analyzer::analyzeField(const DetectedField& detectedField,
                            const vector<const PlayerAnalyzerResult*>& previousResults,
                            PlayerAnalyzerResult* result)
{
    result->detectedField = detectedField;

    // When the previous result is not empty, we use the detected field as is.
    if (previousResults.empty()) {
        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                result->adjustedField.setRealColor(x, y, detectedField.realColor(x, y));
                result->adjustedField.setVanishing(x, y, detectedField.isVanishing(x, y));
            }
        }

        if (detectedField.ojamaDropDetected && !result->hasSentOjamaDropped_) {
            result->userEvent.ojamaDropped = true;
            result->hasSentOjamaDropped_ = true;
        }
        return;
    }

    AdjustedField adjustedField = result->adjustedField;
    {
        // Detecting the current field.
        for (int x = 1; x <= 6; ++x) {
            bool shouldEmpty = false;
            for (int y = 1; y <= 12; ++y) {
                if (shouldEmpty) {
                    adjustedField.field.set(x, y, RealColor::RC_EMPTY);
                    adjustedField.vanishing.set(x, y, false);
                    continue;
                }

                map<RealColor, int> cnt;
                cnt[detectedField.realColor(x, y)]++;
                for (size_t i = 0; i < NUM_FRAMES_TO_SEE_FOR_FIELD && i < previousResults.size(); ++i) {
                    cnt[previousResults[i]->detectedField.realColor(x, y)]++;
                }

                int framesContinuousVanishing = 0;
                if (detectedField.isVanishing(x, y)) {
                    framesContinuousVanishing = 1;
                    for (size_t i = 0; i < NUM_FRAMES_TO_SEE_FOR_FIELD && i < previousResults.size(); ++i) {
                        if (previousResults[i]->detectedField.isVanishing(x, y)) {
                            framesContinuousVanishing += 1;
                        } else {
                            break;
                        }
                    }
                }

                // --- Finds the largest one;
                RealColor rc = RealColor::RC_EMPTY;
                int maxCount = 0;
                for (const auto& entry : cnt) {
                    if (maxCount < entry.second) {
                        rc = entry.first;
                        maxCount = entry.second;
                    }
                }

                adjustedField.field.set(x, y, rc);
                if (rc == RealColor::RC_EMPTY) {
                    shouldEmpty = true;
                    adjustedField.vanishing.set(x, y, false);
                } else {
                    adjustedField.vanishing.set(x, y, (framesContinuousVanishing > 2));
                }
            }
        }
    }

    // We update the field only when
    //  (1) Next1 has disppear
    //  (2) Ojama Dropping has started
    //  (3) Vanishing has started
    // because there won't be no moving puyos.
    // Furthermore, the field is generally stable for several frames.
    // However, there might be several obstacles e.g. sousai-balloon, rensa-effect, or zenkeshi display.
    // It's hard to detect zenkeshi display. However other obstacles are generally moving,
    // so we can detect using the several previous frames.

    bool shouldUpdateField = false;

    // If # of disappearing puyo >= 4, vanishing has started.
    // We should not check before first hand appears.
    int numVanishing = countVanishing(adjustedField.field, adjustedField.vanishing);
    if (numVanishing >= 4) {
        if (!result->hasDetectedRensaStart_) {
            LOG(INFO) << "should update field since vanishing detected.";
            result->playable = false;
            result->hasDetectedRensaStart_ = true;
            shouldUpdateField = true;
        }

        if (!result->hasDetectedPuyoErase_) {
            result->userEvent.puyoErased = true;
        }
        result->hasDetectedPuyoErase_ = true;
    } else {
        result->hasDetectedPuyoErase_ = false;
    }

    if (detectedField.ojamaDropDetected) {
        LOG(INFO) << "ojama dropping detected";
        result->playable = false;
        if (!result->hasSentOjamaDropped_) {
            result->userEvent.ojamaDropped = true;
            result->hasSentOjamaDropped_ = true;
            result->nextWillDisappearFast_ = true;
            // Since this is to analyze field, NEXT has already been analyzed.
            bool axisAndChildsAreEmpty =
                result->detectedField.realColor(NextPuyoPosition::NEXT1_AXIS) == RealColor::RC_EMPTY &&
                result->detectedField.realColor(NextPuyoPosition::NEXT1_CHILD) == RealColor::RC_EMPTY;
            if (axisAndChildsAreEmpty) {
                result->nextHasDisappearedIrregularly_ = true;
            }
        }
    }

    bool shouldResetCurrentState = false;
    if (result->nextPuyoState == NextPuyoState::NEXT2_WILL_DISAPPEAR &&
        previousResults.front()->nextPuyoState == NextPuyoState::STABLE) {
        LOG(INFO) << "should update field since next puyo detected";
        shouldUpdateField = true;
        shouldResetCurrentState = true;
    }

    if (result->nextHasDisappearedIrregularly_) {
        if (detectedField.ojamaDropDetected) {
            result->framesAfterFloorGetsStable_ = 0;
        } else {
            result->framesAfterFloorGetsStable_++;
        }

        if (result->framesAfterFloorGetsStable_ > 3) {
            shouldUpdateField = true;
            shouldResetCurrentState = true;
            result->userEvent.decisionRequestAgain = true;
        }
    }

    if (!shouldUpdateField)
        return;

    if (!result->hasSentGrounded_) {
        result->hasSentGrounded_ = true;
        result->userEvent.grounded = true;
    }

    if (shouldResetCurrentState) {
        result->resetCurrentPuyoState(false);
    }

    // Commit the adjustedField.
    result->adjustedField = adjustedField;
}

void Analyzer::analyzeFieldForLevelSelect(const DetectedField& detectedField, PlayerAnalyzerResult* result)
{
    result->detectedField = detectedField;

    // On LevelSelect state, the whole field should be EMPTY.
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            result->adjustedField.setRealColor(x, y, RealColor::RC_EMPTY);
            result->adjustedField.setVanishing(x, y, false);
        }
    }
}

int Analyzer::countVanishing(const RealColorField& field, const FieldBitField& vanishing)
{
    int result = 0;
    FieldBitField visited;

    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (visited.get(x, y))
                continue;

            if (!vanishing.get(x, y) || !isNormalColor(field.get(x, y))) {
                visited.set(x, y, true);
                continue;
            }

            int cnt = 0;
            queue<pair<int, int>> q;
            q.push(make_pair(x, y));
            while (!q.empty()) {
                int xx = q.front().first;
                int yy = q.front().second;
                q.pop();
                if (visited.get(xx, yy) || !vanishing.get(xx, yy))
                    continue;
                if (field.get(x, y) != field.get(xx, yy))
                    continue;
                ++cnt;
                visited.set(xx, yy, true);
                q.push(make_pair(xx + 1, yy));
                q.push(make_pair(xx - 1, yy));
                q.push(make_pair(xx, yy + 1));
                q.push(make_pair(xx, yy - 1));
            }

            if (cnt >= 4)
                result += cnt;
        }
    }

    return result;
}
