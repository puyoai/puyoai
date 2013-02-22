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
    FeasibleRensaInfo(const BasicRensaInfo& rensaInfo, int initiatingFrames)
        : rensaInfo(rensaInfo), initiatingFrames(initiatingFrames) {}

    BasicRensaInfo rensaInfo;
    int initiatingFrames;
};

// ある状態のフィールドから、いくつかのぷよを追加することで発火することが可能な連鎖
struct PossibleRensaInfo {
    BasicRensaInfo rensaInfo;
    PuyoSet necessaryPuyoSet;

    std::string toString() const {
        return rensaInfo.toString() + necessaryPuyoSet.toString();
    }
};

struct TrackedPossibleRensaInfo {
    BasicRensaInfo rensaInfo;
    PuyoSet necessaryPuyoSet;
    TrackResult trackResult;
};

struct OngoingRensaInfo {
    OngoingRensaInfo() {}
    OngoingRensaInfo(BasicRensaInfo rensaInfo, int finishingRensaFrame)
        : rensaInfo(rensaInfo), finishingRensaFrame(finishingRensaFrame) {}

    BasicRensaInfo rensaInfo;
    int finishingRensaFrame;
};

#endif
