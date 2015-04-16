#ifndef CORE_PLAYER_STATE_H_
#define CORE_PLAYER_STATE_H_

#include "core/core_field.h"
#include "core/rensa_result.h"
#include "core/kumipuyo_seq.h"

struct PlayerStateBase {
    int hand = 0;

    CoreField field;
    KumipuyoSeq seq;

    bool hasZenkeshi = false;
    int fixedOjama = 0;
    int pendingOjama = 0;
};

struct PlayerState : public PlayerStateBase {
    void clear() { *this = PlayerState(); }

    bool isRensaOngoing = false;
    int finishingRensaFrameId = 0;
    RensaResult ongoingRensaResult;

    // make false in decisionRequest. If false twice, fixedOjama should be 0.
    bool hasOjamaDropped = false;
};

struct EnemyState : public PlayerStateBase {
    void clear() { *this = EnemyState(); }

    bool isRensaOngoing = false;
    int finishingRensaFrameId = 0;
    RensaResult ongoingRensaResult;

    // make false in decisionRequest. If false twice, fixedOjama should be 0.
    bool hasOjamaDropped = false;
};

#endif
