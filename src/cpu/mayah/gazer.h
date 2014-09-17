#ifndef GAZER_H_
#define GAZER_H_

#include <map>
#include <string>
#include <vector>

#include "core/algorithm/rensa_info.h"

class KumipuyoSeq;

struct OngoingRensaInfo {
    OngoingRensaInfo() {}
    OngoingRensaInfo(const RensaResult& rensaResult, int finishingRensaFrameId) :
        rensaResult(rensaResult), finishingRensaFrameId(finishingRensaFrameId) {}

    RensaResult rensaResult;
    int finishingRensaFrameId;
};

struct EstimatedRensaInfo {
    EstimatedRensaInfo() {}
    EstimatedRensaInfo(int chains, int score, int initiatingFrames) :
        chains(chains), score(score), initiatingFrames(initiatingFrames)
    {
    }

    std::string toString() const
    {
        char buf[80];
        sprintf(buf, "frames, chains, score = %d, %d, %d", initiatingFrames, chains, score);
        return buf;
    }

    int chains;
    int score;
    int initiatingFrames;
};

class Gazer : noncopyable {
public:
    void initialize(int frameIdGameWillBegin);

    void setFrameIdGazedAt(int frameId) { frameIdGazedAt_ = frameId; }
    int frameIdGazedAt() const { return frameIdGazedAt_; }

    void setOngoingRensa(const OngoingRensaInfo&);
    void unsetOngoingRensa() { isRensaOngoing_ = false; }
    bool isRensaOngoing() const { return isRensaOngoing_; }
    const OngoingRensaInfo& ongoingRensaInfo() const { return ongoingRensaInfo_; }

    void gaze(int frameId, const CoreField&, const KumipuyoSeq&);

    // Returns the (expecting) possible max score by this frame.
    int estimateMaxScore(int frameId) const;

    std::string toRensaInfoString() const;

private:
    int estimateMaxScoreFromFeasibleRensas(int frameId) const;
    int estimateMaxScoreFromPossibleRensas(int frameId) const;
    int estimateMaxScoreFrom(int frameId, const std::vector<EstimatedRensaInfo>& rensaInfos) const;

    void updateFeasibleRensas(const CoreField&, const KumipuyoSeq&);
    void updatePossibleRensas(const CoreField&, const KumipuyoSeq&);

    const std::vector<EstimatedRensaInfo>& possibleRensaInfos() const { return possibleRensaInfos_; }
    const std::vector<EstimatedRensaInfo>& feasibleRensaInfos() const { return feasibleRensaInfos_; }

    int frameIdGazedAt_;
    bool isRensaOngoing_;
    OngoingRensaInfo ongoingRensaInfo_;

    // --- For these rensaInfos, frames means the initiatingFrames.
    // Fiesible Rensa is the Rensa the enemy can really fire in current/next/nextnext tsumo.
    std::vector<EstimatedRensaInfo> feasibleRensaInfos_;
    // Possible Rensa is the Rensa the enemy will build in future.
    std::vector<EstimatedRensaInfo> possibleRensaInfos_;
};

#endif
