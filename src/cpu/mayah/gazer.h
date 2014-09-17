#ifndef GAZER_H_
#define GAZER_H_

#include <map>
#include <string>
#include <vector>

#include "core/algorithm/rensa_info.h"

class KumipuyoSeq;

struct OngoingRensaInfo {
    OngoingRensaInfo() {}
    OngoingRensaInfo(const RensaResult& rensaResult, int finishingRensaFrame) :
        rensaResult(rensaResult), finishingRensaFrame(finishingRensaFrame) {}

    RensaResult rensaResult;
    int finishingRensaFrame;
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
    void initializeWith(int id) {
        setId(id);
        setRensaIsOngoing(false);
        m_feasibleRensaInfos.clear();
        m_possibleRensaInfos.clear();
    }

    void setId(int id) { m_id = id; }
    int id() const { return m_id; }

    void setRensaIsOngoing(bool ongoing) { m_rensaIsOngoing = ongoing; }
    void setOngoingRensa(const OngoingRensaInfo&);
    bool rensaIsOngoing() const { return m_rensaIsOngoing; }
    const OngoingRensaInfo& ongoingRensaInfo() const { return m_ongoingRensaInfo; }

    void updateFeasibleRensas(const CoreField&, const KumipuyoSeq&);
    void updatePossibleRensas(const CoreField&, const KumipuyoSeq&);

    // Returns the (expecting) possible max score by this frame.
    int estimateMaxScore(int frameId) const;
    int estimateMaxScoreFromFeasibleRensas(int frameId) const;
    int estimateMaxScoreFromPossibleRensas(int frameId) const;

    const std::vector<EstimatedRensaInfo>& possibleRensaInfos() const { return m_possibleRensaInfos; }
    const std::vector<EstimatedRensaInfo>& feasibleRensaInfos() const { return m_feasibleRensaInfos; }

    std::string toRensaInfoString() const;

private:
    int estimateMaxScoreFrom(int frameId, const std::vector<EstimatedRensaInfo>& rensaInfos) const;

    int m_id;
    bool m_rensaIsOngoing;
    OngoingRensaInfo m_ongoingRensaInfo;

    // --- For these rensaInfos, frames means the initiatingFrames.
    // Fiesible Rensa is the Rensa the enemy can really fire in current/next/nextnext tsumo.
    std::vector<EstimatedRensaInfo> m_feasibleRensaInfos;
    // Possible Rensa is the Rensa the enemy will build in future.
    std::vector<EstimatedRensaInfo> m_possibleRensaInfos;
};

inline void Gazer::setOngoingRensa(const OngoingRensaInfo& info)
{
     m_rensaIsOngoing = true;
     m_ongoingRensaInfo = info;
}

#endif
