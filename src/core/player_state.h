#ifndef CORE_PLAYER_STATE_H_
#define CORE_PLAYER_STATE_H_

#include "core/core_field.h"
#include "core/rensa_result.h"
#include "core/kumipuyo_seq.h"

struct PlayerState {
    void clear() { *this = PlayerState(); }
    bool isRensaOngoing() const { return currentChainStartedFrameId > 0; }

    int rensaFinishingFrameId() const {
        return currentChainStartedFrameId + currentRensaResult.frames;
    }

    // totalOjama() returns fixed + pending + ongoing.
    int totalOjama(const PlayerState& enemy) const {
        return fixedOjama + pendingOjama + (enemy.currentRensaResult.score + enemy.unusedScore) / 70;
    }

    // noticedOjama() returns fixed + pending.
    // This is almost the same as YOKOKU ojama in your field.
    // If enemy's rensa is ongoing, and you want to count it, what you should use is totalOjama().
    int noticedOjama() const {
        return fixedOjama + pendingOjama;
    }

    int hand = 0;

    CoreField field;
    KumipuyoSeq seq;

    bool hasZenkeshi = false;
    int fixedOjama = 0;
    int pendingOjama = 0;

    int unusedScore = 0; // Score that is not converted to ojama-puyo.
    int currentChain = 0;
    int currentChainStartedFrameId = 0;
    RensaResult currentRensaResult;
    CoreField fieldWhenGrounded;

    // make false in decisionRequest. If false twice, fixedOjama should be 0.
    bool hasOjamaDropped = false;
};

#endif
