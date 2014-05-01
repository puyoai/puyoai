#ifndef __ENEMY_INFO_H_
#define __ENEMY_INFO_H_

#include <map>
#include <string>
#include <vector>

#include "rensa_info.h"

struct EstimatedRensaInfo {
    EstimatedRensaInfo() {}
    EstimatedRensaInfo(int chains, int score, int initiatingFrames)
        : chains(chains), score(score), initiatingFrames(initiatingFrames) {}

    std::string toString() const {
        char buf[80];
        sprintf(buf, "frames, chains, score = %d, %d, %d", initiatingFrames, chains, score);
        return buf;        
    }
        
    int chains;
    int score;
    int initiatingFrames;
};

class EnemyInfo {
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

    void updateFeasibleRensas(const Field&, const std::vector<KumiPuyo>& kumiPuyos);
    void updatePossibleRensas(const Field&, const std::vector<KumiPuyo>& kumiPuyos);

    // Returns the (expecting) possible max score by this frame.
    int estimateMaxScore(int frameId) const;
    int estimateMaxScoreFromFeasibleRensas(int frameId) const;
    int estimateMaxScoreFromPossibleRensas(int frameId) const;

    const std::vector<EstimatedRensaInfo>& possibleRensaInfos() const { return m_possibleRensaInfos; }
    const std::vector<EstimatedRensaInfo>& feasibleRensaInfos() const { return m_feasibleRensaInfos; }

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

inline void EnemyInfo::setOngoingRensa(const OngoingRensaInfo& info)
{
     m_rensaIsOngoing = true;
     m_ongoingRensaInfo = info;
}

#endif
