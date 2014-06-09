#ifndef __PLAYER_INFO_H_
#define __PLAYER_INFO_H_

#include "cpu/mayah/field.h"
#include "core/algorithm/rensa_info.h"

class Decision;
class EvaluationParams;

class MyPlayerInfo {
public:
    const Field& estimatedField() const { return m_estimatedField; }

    void initialize();

    // For field estimation
    void puyoDropped(const Decision&, const Kumipuyo&);
    void ojamaDropped(const Field&);
    void rensaFinished(const Field&);
    void forceEstimatedField(const CoreField& field) { m_estimatedField = field; }

private:
    ArbitrarilyModifiableField m_estimatedField;
};

#endif
