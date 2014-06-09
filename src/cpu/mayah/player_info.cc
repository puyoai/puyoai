#include "player_info.h"

#include <algorithm>
#include <iostream>
#include <glog/logging.h>
#include "core/algorithm/puyo_possibility.h"
#include "core/algorithm/rensa_detector.h"
#include "evaluation_feature.h"

using namespace std;

void MyPlayerInfo::initialize()
{
    m_estimatedField = Field();
}

void MyPlayerInfo::puyoDropped(const Decision& decision, const Kumipuyo& kumiPuyo)
{
    m_estimatedField.dropKumipuyoSafely(decision, kumiPuyo);
    m_estimatedField.simulate();
}

void MyPlayerInfo::ojamaDropped(const Field& field)
{
    // TODO: もう少し正確にするべき

    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = 1; y <= Field::HEIGHT; ++y)
            m_estimatedField.setPuyo(x, y, field.color(x, y));

        for (int y = Field::HEIGHT + 1; y <= 14; ++y)
            m_estimatedField.setPuyo(x, y, EMPTY);

        m_estimatedField.recalcHeightOn(x);
        if (m_estimatedField.color(x, Field::HEIGHT) != EMPTY) {
            while (m_estimatedField.height(x) < 13)
                m_estimatedField.dropPuyoOn(x, OJAMA);
        }
    }
}

void MyPlayerInfo::rensaFinished(const Field& field)
{
    // TODO: もう少し正確にするべき
    ojamaDropped(field);
}
