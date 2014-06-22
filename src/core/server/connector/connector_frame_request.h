#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_FRAME_REQUEST_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_FRAME_REQUEST_H_

#include <string>
#include <vector>

#include "core/game_result.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"
#include "core/state.h"

struct ConnectorFrameRequest {
    std::string toRequestString(int playerId) const;

    int frameId = -1;
    GameResult gameResult;
    PlainField field[2];
    Kumipuyo kumipuyo[2][3]; // 0 = current, 1 = next1, 2 = next2
    KumipuyoPos kumipuyoPos[2];
    UserState userState[2];
    int score[2];
    int ojama[2];
    int ackFrameId[2];
    std::vector<int> nackFrameIds[2];
};

#endif
