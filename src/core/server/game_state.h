#ifndef CORE_SERVER_GAME_STATE_H_
#define CORE_SERVER_GAME_STATE_H_

#include <string>
#include <glog/logging.h>

#include "base/base.h"
#include "core/decision.h"
#include "core/frame_request.h"
#include "core/game_result.h"

struct PlayerGameState {
    int ojama() const { return pendingOjama + fixedOjama; }

    PlainField field;
    KumipuyoSeq kumipuyoSeq;
    KumipuyoPos kumipuyoPos;
    UserState state;
    bool dead;
    bool playable;
    int score;
    int pendingOjama;
    int fixedOjama;
    Decision decision;
    std::string message;
};

class GameState {
public:
    explicit GameState(int frameId) : frameId_(frameId) {}

    FrameRequest toFrameRequestFor(int playerId) const;
    FrameRequest toFrameRequestFor(int playerId, GameResult forceSetGameResult) const;

    int frameId() const { return frameId_; }

    std::string toJson() const;
    std::string toDebugString() const;

    GameResult gameResult() const;

    const PlayerGameState& playerGameState(int playerId) const { return playerGameState_[playerId]; }
    PlayerGameState* mutablePlayerGameState(int playerId) {  return &playerGameState_[playerId]; }

private:
    int frameId_ = 0;
    PlayerGameState playerGameState_[2];
};

#endif
