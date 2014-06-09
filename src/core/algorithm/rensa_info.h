#ifndef CORE_ALGORITHM_RENSA_INFO_H_
#define CORE_ALGORITHM_RENSA_INFO_H_

#include <string>
#include "core/field/rensa_result.h"
#include "core/algorithm/column_puyo_list.h"

struct FeasibleRensaInfo {
    FeasibleRensaInfo() {}
    FeasibleRensaInfo(const BasicRensaResult& rensaInfo, int initiatingFrames)
        : basicRensaResult(rensaInfo), initiatingFrames(initiatingFrames) {}

    BasicRensaResult basicRensaResult;
    int initiatingFrames;
};

struct PossibleRensaInfo {
    BasicRensaResult rensaInfo;
    ColumnPuyoList necessaryPuyoSet;

    std::string toString() const {
        return rensaInfo.toString() + necessaryPuyoSet.toString();
    }
};

struct TrackedPossibleRensaInfo {
    BasicRensaResult rensaInfo;
    ColumnPuyoList necessaryPuyoSet;
    RensaTrackResult trackResult;
};

struct OngoingRensaInfo {
    OngoingRensaInfo() {}
    OngoingRensaInfo(const BasicRensaResult& rensaInfo, int finishingRensaFrame) :
        rensaInfo(rensaInfo), finishingRensaFrame(finishingRensaFrame) {}

    BasicRensaResult rensaInfo;
    int finishingRensaFrame;
};

#endif
