#ifndef __PLAYER_INFO_H_
#define __PLAYER_INFO_H_

#include "field.h"
#include "rensa_info.h"

class Decision;
class EvaluationParams;
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

    void updateMainRensa(const std::vector<KumiPuyo>&, const EvaluationParams&);

    const TrackResult& mainRensaTrackResult() const { return m_mainRensaTrackResult; }
    int mainRensaChains() const { return m_mainRensaChains; }
    double mainRensaHandWidth() const { return m_mainRensaHandWidth; }
    int mainRensaDistanceCount(int nth) const { return m_mainRensaDistanceCount[nth]; }

private:
    ArbitrarilyModifiableField m_estimatedField;

    TrackResult m_mainRensaTrackResult;
    int m_mainRensaChains;
    double m_mainRensaHandWidth;
    int m_mainRensaDistanceCount[5];
};

#endif
