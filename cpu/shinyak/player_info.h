#ifndef __PLAYER_INFO_H_
#define __PLAYER_INFO_H_

#include <map>
#include <string>
#include <vector>

#include "rensainfo.h"

class Field;
class KumiPuyo;

class MyPlayerInfo {
public:
    const Field& estimatedField() const { return m_estimatedField; }

    void initialize();

    void puyoDropped(const Decision&, const KumiPuyo&);
    void ojamaDropped(const Field&);
    void rensaFinished(const Field&);

    void forceEstimatedField(const Field& field) { m_estimatedField = field; }

private:
    Field m_estimatedField;
};

struct OngoingRensaInfo {
    OngoingRensaInfo() {}
    OngoingRensaInfo(BasicRensaInfo rensaInfo, int finishingRensaFrame)
        : rensaInfo(rensaInfo), finishingRensaFrame(finishingRensaFrame) {}

    BasicRensaInfo rensaInfo;
    int finishingRensaFrame;
};

class EnemyInfo {
public:
    void initializeWith(int id) {
        setId(id);
        setRensaIsOngoing(false);
        m_emptyFieldAvailability = 0;
        m_feasibleRensaInfos.clear();
        m_possibleRensaInfos.clear();
    }

    void setId(int id) { m_id = id; }
    int id() const { return m_id; }

    void setRensaIsOngoing(bool ongoing) { m_rensaIsOngoing = ongoing; }
    void setOngoingRensa(const OngoingRensaInfo&);
    bool rensaIsOngoing() const { return m_rensaIsOngoing; }
    const OngoingRensaInfo& ongoingRensaInfo() const { return m_ongoingRensaInfo; }

    void setEmptyFieldAvailability(double availability) { m_emptyFieldAvailability = availability; }
    double emptyFieldAvailability() const { return m_emptyFieldAvailability; }

    void updateFeasibleRensas(const Field&, const std::vector<KumiPuyo>& kumiPuyos);
    void updatePossibleRensas(const Field&, const std::vector<KumiPuyo>& kumiPuyos);

    // Returns the (expecting) possible max score by this frame.
    int estimateMaxScore(int frameId) const;
    int estimateMaxScoreFromFeasibleRensas(int frameId) const;
    int estimateMaxScoreFromPossibleRensas(int frameId) const;

    const std::vector<BasicRensaInfo>& possibleRensaInfos() const { return m_possibleRensaInfos; }
    const std::vector<BasicRensaInfo>& feasibleRensaInfos() const { return m_feasibleRensaInfos; }

private:
    int estimateMaxScoreFrom(int frameId, const std::vector<BasicRensaInfo>& rensaInfos) const;

    int m_id;

    double m_emptyFieldAvailability;

    bool m_rensaIsOngoing;
    OngoingRensaInfo m_ongoingRensaInfo;

    // --- For these rensaInfos, frames means the initiatingFrames.
    // Fiesible Rensa is the Rensa the enemy can really fire in current/next/nextnext tsumo.
    std::vector<BasicRensaInfo> m_feasibleRensaInfos;
    // Possible Rensa is the Rensa the enemy will build in future.
    std::vector<BasicRensaInfo> m_possibleRensaInfos;
};

inline void EnemyInfo::setOngoingRensa(const OngoingRensaInfo& info)
{
     m_rensaIsOngoing = true;
     m_ongoingRensaInfo = info;
}

#endif
