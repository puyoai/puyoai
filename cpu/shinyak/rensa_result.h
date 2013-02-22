#ifndef __RENSA_RESULT_H_
#define __RENSA_RESULT_H_

#include <string>
#include "field.h"
#include "util.h"

struct BasicRensaInfo {
    BasicRensaInfo() 
        : chains(0), score(0), frames(0) {}
    BasicRensaInfo(int chains, int score, int frames)
        : chains(chains), score(score), frames(frames) {}

    std::string toString() const;

    int chains;
    int score;
    int frames;
};

class TrackResult {
public:
    TrackResult& operator=(const TrackResult& result);

    // Nth Rensa where (x, y) is erased. 0 if not erased.
    int erasedAt(int x, int y) const { return m_erasedAt[x][y]; }
    void setErasedAt(int x, int y, int nthChain) { m_erasedAt[x][y] = nthChain; }

    std::string toString() const;

private:
    byte m_erasedAt[Field::MAP_WIDTH][Field::MAP_HEIGHT];
};

struct TrackedRensaInfo {
    TrackedRensaInfo() {}
    TrackedRensaInfo(const BasicRensaInfo& rensaInfo, const TrackResult& trackResult)
        : rensaInfo(rensaInfo), trackResult(trackResult) {}

    BasicRensaInfo rensaInfo;
    TrackResult trackResult;
};

#endif
