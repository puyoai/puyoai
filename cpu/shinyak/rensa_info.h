#ifndef __RENSA_INFO_H_
#define __RENSA_INFO_H_

#include <string>
#include "field.h"
#include "puyo_set.h"
#include "rensa_result.h"

// TODO(mayah): These RensaInfo is difficult to use sometimes. Refactoring is desired.

// CURRENT/NEXT/NEXTNEXT から実際に発火可能な連鎖 (For PlayerInfo)
struct FeasibleRensaInfo {
    FeasibleRensaInfo() {}
    FeasibleRensaInfo(const BasicRensaResult& rensaInfo, int initiatingFrames)
        : basicRensaResult(rensaInfo), initiatingFrames(initiatingFrames) {}

    BasicRensaResult basicRensaResult;
    int initiatingFrames;
};

// ある状態のフィールドから、いくつかのぷよを追加することで発火することが可能な連鎖
struct PossibleRensaInfo {
    BasicRensaResult rensaInfo;
    PuyoSet necessaryPuyoSet;

    std::string toString() const {
        return rensaInfo.toString() + necessaryPuyoSet.toString();
    }
};

struct TrackedPossibleRensaInfo {
    BasicRensaResult rensaInfo;
    PuyoSet necessaryPuyoSet;
    RensaTrackResult trackResult;
};

struct OngoingRensaInfo {
    OngoingRensaInfo() {}
    OngoingRensaInfo(const BasicRensaResult& rensaInfo, int finishingRensaFrame)
        : rensaInfo(rensaInfo), finishingRensaFrame(finishingRensaFrame) {}

    BasicRensaResult rensaInfo;
    int finishingRensaFrame;
};

#endif
