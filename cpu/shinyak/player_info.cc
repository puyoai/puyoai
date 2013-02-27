#include "player_info.h"

#include <algorithm>
#include <glog/logging.h>
#include <iostream>
#include "evaluation_feature.h"
#include "puyo_possibility.h"
#include "rensa_detector.h"

using namespace std;

// TODO(mayah): Maybe this function should be moved to an appropriate file.
// I'm not sure where it is though...
// Maybe FieldEvaluator?
static double calculateHandWidth(int distanceCountResult[5], const TrackResult& trackResult, const Field& field, const EvaluationParams& params)
{
    for (int i = 0; i < 5; ++i)
        distanceCountResult[i] = 0;

    // 1 連鎖の部分を距離 1 とし、距離 4 までを求める。
    // 距離 2, 3, 4 の数を数え、その広がり具合により、手の広さを求めることができる。
    int distance[Field::MAP_WIDTH][Field::MAP_HEIGHT];
    for (int x = 0; x < Field::MAP_WIDTH; ++x) {
        for (int y = 0; y < Field::MAP_HEIGHT; ++y) {
            if (trackResult.erasedAt(x, y) == 1)
                distance[x][y] = 1;
            else
                distance[x][y] = 0;
        }
    }

    for (int d = 2; d <= 4; ++d) {
        for (int x = 1; x <= Field::WIDTH; ++x) {
            for (int y = 1; y <= Field::HEIGHT; ++y) {
                if (field.color(x, y) != EMPTY || distance[x][y] > 0)
                    continue;
                if (distance[x][y-1] == d - 1 || distance[x][y+1] == d - 1 || distance[x-1][y] == d - 1 || distance[x+1][y] == d - 1) {
                    distance[x][y] = d;
                    ++distanceCountResult[d];
                }
            }
        }
    }

    return params.calculateHandWidthScore(distanceCountResult[1], distanceCountResult[2], distanceCountResult[3], distanceCountResult[4]);
}

void MyPlayerInfo::initialize()
{
    m_estimatedField = Field();
    m_mainRensaChains = 0;
    memset(m_mainRensaDistanceCount, 0, sizeof(m_mainRensaDistanceCount));
}

void MyPlayerInfo::puyoDropped(const Decision& decision, const KumiPuyo& kumiPuyo)
{
    m_estimatedField.dropKumiPuyoSafely(decision, kumiPuyo);
}

void MyPlayerInfo::ojamaDropped(const Field& field)
{
    // TODO: もう少し正確にするべき
    
    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = 1; y <= Field::HEIGHT; ++y)
            m_estimatedField.setColor(x, y, field.color(x, y));

        for (int y = Field::HEIGHT + 1; y <= 14; ++y)
            m_estimatedField.setColor(x, y, EMPTY);

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

void MyPlayerInfo::updateMainRensa(const vector<KumiPuyo>& kumiPuyos, const EvaluationParams& params)
{
    DCHECK(kumiPuyos.size() == 2);

    vector<TrackedPossibleRensaInfo> results;
    RensaDetector::findPossibleRensas(results, estimatedField(), 3);

    if (results.empty()) {
        m_mainRensaTrackResult = TrackResult();
        m_mainRensaChains = 0;
        return;
    }

    // 次のようなものを MainRensa として選択する
    // 1. わかっている最大連鎖-1 以上を持つ
    // 2. その中で、最も手が広いものを選択する
    // 3. 手が広いものが複数あれば、possibility を評価する
    //   3.1. ここで、possibility を評価する場合には、kumiPuyos を subtract してよい。

    int maxChains = 0;
    for (auto it = results.begin(); it != results.end(); ++it) {
        if (maxChains < it->rensaInfo.chains)
            maxChains = it->rensaInfo.chains;
    }

    m_mainRensaChains = maxChains;
    LOG(INFO) << "Max chains is " << maxChains << endl;

    PuyoSet kumiPuyoSet;
    for (auto it = kumiPuyos.begin(); it != kumiPuyos.end(); ++it) {
        kumiPuyoSet.add(it->axis);
        kumiPuyoSet.add(it->child);
    }

    double maxHandWidth = 0;
    double maxPossibility = 0;
    int maxDistanceCountResult[5] = { 0, 0, 0, 0, 0 };
    auto maxRensaIter = results.begin();
    for (auto it = results.begin(); it != results.end(); ++it) {
        int chains = it->rensaInfo.chains;
        if (chains < maxChains)
            continue;

        PuyoSet necessarySet(it->necessaryPuyoSet);
        necessarySet.sub(kumiPuyoSet);
        int distanceCountResult[5];
        double handWidth = calculateHandWidth(distanceCountResult, it->trackResult, estimatedField(), params);
        double possibility = TsumoPossibility::possibility(4, necessarySet);

        if (maxHandWidth < handWidth) {
            maxHandWidth = handWidth;
            maxPossibility = possibility;
            maxRensaIter = it;
            memmove(maxDistanceCountResult, distanceCountResult, sizeof(distanceCountResult));
        } else if (maxHandWidth == handWidth && maxPossibility < possibility) {
            maxPossibility = possibility;
            maxRensaIter = it;            
            memmove(maxDistanceCountResult, distanceCountResult, sizeof(distanceCountResult));
        }        
    }

    m_mainRensaHandWidth = maxHandWidth;
    m_mainRensaChains = maxRensaIter->rensaInfo.chains;
    m_mainRensaTrackResult = maxRensaIter->trackResult;
    memmove(m_mainRensaDistanceCount, maxDistanceCountResult, sizeof(maxDistanceCountResult));
}
