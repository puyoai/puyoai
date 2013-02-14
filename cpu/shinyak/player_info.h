#ifndef __PLAYER_INFO_H_
#define __PLAYER_INFO_H_

#include "field.h"
#include "rensa_info.h"

class Decision;
class KumiPuyo;

class MyPlayerInfo {
public:
    const Field& estimatedField() const { return m_estimatedField; }

    void initialize();

    // For field estimation
    void puyoDropped(const Decision&, const KumiPuyo&);
    void ojamaDropped(const Field&);
    void rensaFinished(const Field&);
    void forceEstimatedField(const Field& field) { m_estimatedField = field; }

    void updateMainRensa(const std::vector<KumiPuyo>&);
    const TrackResult& mainRensaTrackResult() const { return m_mainRensaTrackResult; }
    int mainRensaChains() const { return m_mainRensaChains; }

private:
    Field m_estimatedField;

    TrackResult m_mainRensaTrackResult;
    int m_mainRensaChains;
};

#endif
