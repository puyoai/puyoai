#ifndef CORE_FRAME_DATA_H_
#define CORE_FRAME_DATA_H_

#include <string>
#include <vector>

#include "core/plain_field.h"
#include "core/kumipuyo.h"
#include "core/state.h"

struct PlayerFrameData {
    PlayerFrameData() {}
    PlayerFrameData(const PlainField&, const KumipuyoSeq&, const KumipuyoPos&, const UserState&, int score, int ojama);

    PlainField field;
    KumipuyoSeq kumipuyoSeq;
    KumipuyoPos kumipuyoPos;
    UserState userState;
    int score = 0;
    int ojama = 0;
};

struct FrameData {
    static const int GAME_END_UNDEFINED = -255;

    FrameData() {}

    const PlayerFrameData& myPlayerFrameData() const { return playerFrameData[0]; }
    const PlayerFrameData& enemyPlayerFrameData() const { return playerFrameData[1]; }

    bool hasGameEnd() const { return gameEnd != GAME_END_UNDEFINED; }
    std::string toString() const;

    bool connectionLost = false;
    bool valid = false;
    int id = -1;
    int gameEnd = GAME_END_UNDEFINED;
    PlayerFrameData playerFrameData[2];  // 0 = me, 1 = opponent
};

#endif
