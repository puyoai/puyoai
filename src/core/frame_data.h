#ifndef CORE_FRAME_DATA_H_
#define CORE_FRAME_DATA_H_

#include <vector>

#include "core/plain_field.h"
#include "core/kumipuyo.h"
#include "core/state.h"

struct PlayerFrameData {
    PlayerFrameData();
    PlayerFrameData(const PlainField&, const KumipuyoSeq&, const KumipuyoPos&, const UserState&, int score, int ojama);

    PlainField field;
    KumipuyoSeq kumipuyoSeq;
    KumipuyoPos kumipuyoPos;
    UserState userState;
    int score;
    int ojama;
};

struct FrameData {
    FrameData();

    const PlayerFrameData& myPlayerFrameData() const { return playerFrameData[0]; }
    const PlayerFrameData& enemyPlayerFrameData() const { return playerFrameData[1]; }

    bool connectionLost;
    bool valid;
    int id;
    PlayerFrameData playerFrameData[2];  // 0 = me, 1 = opponent
};

#endif
