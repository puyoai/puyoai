#ifndef CORE_FRAME_REQUEST_H_
#define CORE_FRAME_REQUEST_H_

#include <string>
#include <vector>

#include "core/game_result.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"
#include "core/player.h"
#include "core/state.h"

struct PlayerFrameRequest {
    PlainField field;
    KumipuyoSeq kumipuyoSeq;
    KumipuyoPos kumipuyoPos;
    UserState state;
    int score = 0;
    int ojama = 0;
    int ackFrameId = -1;
    std::vector<int> nackFrameIds;
};

struct FrameRequest {
    static FrameRequest parse(const std::string& line);

    std::string toString() const;
    std::string toDebugString() const;

    bool isValid() const { return frameId != -1; }
    bool hasGameEnd() const { return gameResult != GameResult::PLAYING; }

    const PlayerFrameRequest& myPlayerFrameRequest() const { return playerFrameRequest[0]; }
    const PlayerFrameRequest& enemyPlayerFrameRequest() const { return playerFrameRequest[1]; }

    bool connectionLost = false;
    int frameId = -1;
    GameResult gameResult = GameResult::PLAYING;
    PlayerFrameRequest playerFrameRequest[NUM_PLAYERS];
};

#endif
