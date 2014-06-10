#include "capture/analyzer.h"

#include <queue>

#include "core/constant.h"

using namespace std;

namespace {
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

    DCHECK(false) << "Unknown CaptureGameState: "  << static_cast<int>(cgs);
    return "";
}

AdjustedField::AdjustedField()
{
    for (int x = 0; x < 6; ++x) {
        for (int y = 0; y < 12; ++y) {
            puyos[x][y] = RealColor::RC_EMPTY;
            vanishing[x][y] = false;
        }
    }

    for (int i = 0; i < NUM_NEXT_PUYO_POSITION; ++i) {
        nextPuyos[i] = RealColor::RC_EMPTY;
    }
}

string PlayerAnalyzerResult::toString() const
{
    stringstream ss;

    ss << "nextPuyoState: " << static_cast<int>(nextPuyoState) << endl;
    ss << "userState" << userState.toString() << endl;

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

std::unique_ptr<AnalyzerResult> Analyzer::analyze(const SDL_Surface* surface, const deque<unique_ptr<AnalyzerResult>>& previousResults)
{
    auto gameState = detectGameState(surface);
    auto player1FieldResult = detectField(0, surface);
    auto player2FieldResult = detectField(1, surface);

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
    case CaptureGameState::FINISHED: {
        // After finished, we don't need to check each player gamestate.
        auto player1Result = unique_ptr<PlayerAnalyzerResult>();
        auto player2Result = unique_ptr<PlayerAnalyzerResult>();
        return std::unique_ptr<AnalyzerResult>(new AnalyzerResult(gameState, move(player1Result), move(player2Result)));
    }
    case CaptureGameState::PLAYING: {
        auto player1Result = analyzePlayerField(*player1FieldResult, makePlayerOnlyResults(0, previousResults));
        auto player2Result = analyzePlayerField(*player2FieldResult, makePlayerOnlyResults(1, previousResults));
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
        result->userState.clearEventStates();
    }

    analyzeNext(detectedField, previousResults, result.get(), FOR_LEVEL_SELECT);
    analyzeField(detectedField, previousResults, result.get(), FOR_LEVEL_SELECT);

    result->initializeCurrentPuyoState();

    return result;
}

unique_ptr<PlayerAnalyzerResult>
Analyzer::analyzePlayerField(const DetectedField& detectedField, const vector<const PlayerAnalyzerResult*>& previousResults)
{
    unique_ptr<PlayerAnalyzerResult> result(new PlayerAnalyzerResult);
    // Copy the previous state, however, clear event states.
    if (!previousResults.empty()) {
        *result = *previousResults.front();
        result->userState.clearEventStates();
    }

    // Note that Next should be analyzed before Field, since we use the result of the analysis.
    analyzeNext(detectedField, previousResults, result.get(), FOR_RUNNING);
    analyzeField(detectedField, previousResults, result.get(), FOR_RUNNING);

    // --- Finalize the current state
    if (result->restFramesUserCanPlay > 0) {
        result->restFramesUserCanPlay -= 1;
        if (result->restFramesUserCanPlay == 0 && !result->userState.playable)
            result->userState.playable = true;

        // Even if restFramesUserCanPlay_[pi] > 0, a puyo might appear on (3, 12).
        // In that case, we skip the rest of waiting frame.
        if (result->realColor(3, 12) != RC_EMPTY) {
            result->restFramesUserCanPlay = 0;
            result->userState.playable = true;
        }
    }

    return result;
}

void Analyzer::analyzeNext(const DetectedField& detectedField, const vector<const PlayerAnalyzerResult*>& previousResults,
                           PlayerAnalyzerResult* result, Analyzer::ForLevelSelectEnum forLevelSelectEnum)
{
    // When selecting a level, there will not exist any obstracle.
    // So, we don't need to stabilize next2, maybe.
    if (forLevelSelectEnum == FOR_LEVEL_SELECT) {
        return analyzeNextForLevelSelect(detectedField, result);
    }

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
    result->userState.playable = false;
    result->restFramesUserCanPlay = 0;
    result->initializeCurrentPuyoState();

    // There should not exist moving puyos. So, CURRENT_AXIS and CURRENT_CHILD should be empty.
    result->setRealColor(NextPuyoPosition::CURRENT_AXIS, RC_EMPTY);
    result->setRealColor(NextPuyoPosition::CURRENT_CHILD, RC_EMPTY);

    // NOTE: When axisColor or childColor is not normal, we skip this analysis.
    // This is because sometimes we miss-detect the next field.

    // Check NEXT1
    {
        RealColor axisColor = detectedField.realColor(NextPuyoPosition::NEXT1_AXIS);
        RealColor childColor = detectedField.realColor(NextPuyoPosition::NEXT1_CHILD);

        if (isNormalColor(axisColor) && isNormalColor(childColor)) {
            result->setRealColor(NextPuyoPosition::NEXT1_AXIS, axisColor);
            result->setRealColor(NextPuyoPosition::NEXT1_CHILD, childColor);
        }
    }

    // Check NEXT2
    {
        RealColor axisColor = detectedField.realColor(NextPuyoPosition::NEXT2_AXIS);
        RealColor childColor = detectedField.realColor(NextPuyoPosition::NEXT2_CHILD);

        if (isNormalColor(axisColor) && isNormalColor(childColor)) {
            result->setRealColor(NextPuyoPosition::NEXT2_AXIS, axisColor);
            result->setRealColor(NextPuyoPosition::NEXT2_CHILD, childColor);
        }
    }
}

void Analyzer::analyzeNextWhenPreviousResultDoesNotExist(const DetectedField& detectedField, PlayerAnalyzerResult* result)
{
    // We cannot do much thing. Let's consider that the current next state is STABLE.
    // TODO(mayah): Currently we do the same thing as LevelSelect.
    analyzeNextForLevelSelect(detectedField, result);
}

void Analyzer::analyzeNextForStateStable(const DetectedField& detectedField, PlayerAnalyzerResult* result)
{
    // Check Next1 disappears.
    RealColor axisColor = detectedField.realColor(NextPuyoPosition::NEXT1_AXIS);
    RealColor childColor = detectedField.realColor(NextPuyoPosition::NEXT1_CHILD);

    // NEXT1 has not disappeared yet.
    if (axisColor != RC_EMPTY && childColor != RC_EMPTY) {
        result->framesWhileNext1Disappearing = 0;
        return;
    }

    // Detected NEXT1 has disappeard.
    result->framesWhileNext1Disappearing += 1;

    // When NEXT2 is not stabilized, we cannot proceed the current state.
    RealColor next2AxisColor = result->realColor(NextPuyoPosition::NEXT2_AXIS);
    RealColor next2ChildColor = result->realColor(NextPuyoPosition::NEXT2_CHILD);
    if (!isNormalColor(next2AxisColor) || !isNormalColor(next2ChildColor))
        return;

    // We want to see NEXT1 is absent in 2 frames for stability.
    if (result->framesWhileNext1Disappearing < 2)
        return;

    // Detected Next1 disappeared
    result->restFramesUserCanPlay = 2;
    result->nextPuyoState = NextPuyoState::NEXT2_WILL_DISAPPEAR;
    result->userState.playable = false;
    result->setRealColor(NextPuyoPosition::CURRENT_AXIS, result->realColor(NextPuyoPosition::NEXT1_AXIS));
    result->setRealColor(NextPuyoPosition::CURRENT_CHILD, result->realColor(NextPuyoPosition::NEXT1_CHILD));
    result->setRealColor(NextPuyoPosition::NEXT1_AXIS, result->realColor(NextPuyoPosition::NEXT2_AXIS));
    result->setRealColor(NextPuyoPosition::NEXT1_CHILD, result->realColor(NextPuyoPosition::NEXT2_CHILD));
    result->setRealColor(NextPuyoPosition::NEXT2_AXIS, RC_EMPTY);
    result->setRealColor(NextPuyoPosition::NEXT2_CHILD, RC_EMPTY);

    if (result->hasDetectedOjamaDrop_ && !result->hasSentOjamaDropped_) {
        result->userState.ojamaDropped = true;
        result->hasSentOjamaDropped_ = true;
    }
    if (result->hasDetectedRensaStart_ && !result->hasSentChainFinished_) {
        result->userState.chainFinished = true;
        result->hasSentChainFinished_ = true;
    }
}

void Analyzer::analyzeNextForStateNext2WillDisappear(const DetectedField& detectedField, PlayerAnalyzerResult* result)
{
    // Stay until next2 disappears.
    RealColor axisColor = detectedField.realColor(NextPuyoPosition::NEXT2_AXIS);
    RealColor childColor = detectedField.realColor(NextPuyoPosition::NEXT2_CHILD);

    if (axisColor == RC_EMPTY || childColor == RC_EMPTY) {
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
            result->setRealColor(NextPuyoPosition::NEXT2_AXIS, axisColor);
            result->setRealColor(NextPuyoPosition::NEXT2_CHILD, childColor);
            result->nextPuyoState = NextPuyoState::STABLE;
            result->userState.wnextAppeared = true;
            result->next2Puyos.clear();
        }
    }
}

void Analyzer::analyzeField(const DetectedField& detectedField, const vector<const PlayerAnalyzerResult*>& previousResults,
                            PlayerAnalyzerResult* result, ForLevelSelectEnum forLevelSelectEnum)
{
    result->detectedField = detectedField;

    if (forLevelSelectEnum == FOR_LEVEL_SELECT) {
        // On LevelSelect state, the whole field should be EMPTY.
        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                result->setRealColor(x, y, RealColor::RC_EMPTY);
                result->setVanishing(x, y, false);
            }
        }

        return;
    }

    // When the previous result does not empty, we use the detected field as is.
    if (previousResults.empty()) {
        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                result->setRealColor(x, y, detectedField.realColor(x, y));
                result->setVanishing(x, y, detectedField.isVanishing(x, y));
            }
        }
        return;
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

    if (!result->hasDetectedRensaStart_) {
        // If # of disappearing puyo >= 4, vanishing has started.
        // We should not check before first hand appears.
        int numVanishing = countVanishing(result->detectedField.puyos, result->detectedField.vanishing);
        if (numVanishing >= 4) {
            LOG(INFO) << "should update field since vanishing detected";
            result->userState.playable = false;
            result->hasDetectedRensaStart_ = true;
            shouldUpdateField = true;
        }
    }

    if (!result->hasDetectedOjamaDrop_) {
        // When # of ojama puyo is more than expected, maybe ojama dropping started.
        int numOjama = countOjama(result->detectedField.puyos);
        result->numOjama = numOjama;
        if (!previousResults.empty() && numOjama > previousResults.front()->numOjama) {
            LOG(INFO) << "should update field since ojama dropping detected";
            result->userState.playable = false;
            result->hasDetectedOjamaDrop_ = true;
            shouldUpdateField = true;
        }
    }

    if (result->nextPuyoState == NextPuyoState::NEXT2_WILL_DISAPPEAR &&
        previousResults.front()->nextPuyoState == NextPuyoState::STABLE) {
        LOG(INFO) << "should update field since next puyo detected";
        shouldUpdateField = true;

        result->hasDetectedOjamaDrop_ = false;
        result->hasDetectedRensaStart_ = false;
        result->hasSentGrounded_ = false;
        result->hasSentOjamaDropped_ = false;
        result->hasSentChainFinished_ = false;
    }

    if (!shouldUpdateField)
        return;

    if (!result->hasSentGrounded_) {
        result->hasSentGrounded_ = true;
        result->userState.grounded = true;
    }

    // Detecting the current field.
    for (int x = 1; x <= 6; ++x) {
        bool shouldEmpty = false;
        for (int y = 1; y <= 12; ++y) {
            if (shouldEmpty) {
                result->setRealColor(x, y, RealColor::RC_EMPTY);
                continue;
            }

            map<RealColor, int> cnt;
            cnt[detectedField.realColor(x, y)]++;
            for (int i = 0; i < 5 && i < previousResults.size(); ++i) {
                cnt[previousResults[i]->detectedField.realColor(x, y)]++;
            }

            // --- Finds the largest one;
            RealColor rc = RC_EMPTY;
            int maxCount = 0;
            for (const auto& entry : cnt) {
                if (maxCount < entry.second) {
                    rc = entry.first;
                    maxCount = entry.second;
                }
            }

            result->setRealColor(x, y, rc);
            if (rc == RealColor::RC_EMPTY)
                shouldEmpty = true;
        }
    }
}

int Analyzer::countOjama(RealColor puyos[6][12])
{
    int cnt = 0;
    for (int x = 0; x < 6; ++x) {
        for (int y = 0; y < 12; ++y) {
            if (puyos[x][y] == RC_OJAMA)
                ++cnt;
        }
    }

    return cnt;
}

int Analyzer::countVanishing(RealColor puyos[6][12], bool vanishing[6][12])
{
    int result = 0;
    bool visited[6][12] = {{false}};

    for (int x = 0; x < 6; ++x) {
        for (int y = 0; y < 12; ++y) {
            if (visited[x][y])
                continue;

            if (!vanishing[x][y] || puyos[x][y] == RealColor::RC_EMPTY) {
                visited[x][y] = true;
                continue;
            }

            int cnt = 0;
            queue<pair<int, int>> q;
            q.push(make_pair(x, y));
            while (!q.empty()) {
                int xx = q.front().first;
                int yy = q.front().second;
                q.pop();
                if (xx < 0 || 6 <= xx || yy < 0 || 12 <= yy)
                    continue;
                if (visited[xx][yy] || !vanishing[xx][yy])
                    continue;
                ++cnt;
                visited[xx][yy] = true;
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
