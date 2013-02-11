#ifndef __RENSA_INFO_H_
#define __RENSA_INFO_H_

#include <string>
#include "field.h"
#include "puyo_set.h"

class TrackResult {
    friend class TrackingStrategy;
public:
    // Nth Rensa where (x, y) is erased. 0 if not erased.
    int erasedAt(int x, int y) const { return m_erasedAt[x][y]; }

    TrackResult& operator=(const TrackResult& result) {
        for (int x = 0; x < Field::MAP_WIDTH; ++x) {
            for (int y = 0; y < Field::MAP_HEIGHT; ++y)
                m_erasedAt[x][y] = result.m_erasedAt[x][y];
        }

        return *this;
    }

    byte m_erasedAt[Field::MAP_WIDTH][Field::MAP_HEIGHT];
};

struct BasicRensaInfo {
    BasicRensaInfo() 
        : chains(0), score(0), frames(0) {}
    BasicRensaInfo(int chains, int score, int frames)
        : chains(chains), score(score), frames(frames) {}

    std::string toString() const {
        char buf[80];
        sprintf(buf, "chains, score, frames = %d, %d, %d", chains, score, frames);
        return buf;
    }

    int chains;
    int score;
    int frames;
};

struct TrackedRensaInfo {
    TrackedRensaInfo() {}
    TrackedRensaInfo(const BasicRensaInfo& rensaInfo, const TrackResult& trackResult)
        : rensaInfo(rensaInfo), trackResult(trackResult) {}

    BasicRensaInfo rensaInfo;
    TrackResult trackResult;
};

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

struct OngoingRensaInfo {
    OngoingRensaInfo() {}
    OngoingRensaInfo(BasicRensaInfo rensaInfo, int finishingRensaFrame)
        : rensaInfo(rensaInfo), finishingRensaFrame(finishingRensaFrame) {}

    BasicRensaInfo rensaInfo;
    int finishingRensaFrame;
};

#endif
