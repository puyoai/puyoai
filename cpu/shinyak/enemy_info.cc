#include "enemy_info.h"

#include <algorithm>
#include <glog/logging.h>
#include <iostream>
#include "../../core/constant.h"
#include "rensa_detector.h"
#include "rensa_info.h"
#include "puyo_possibility.h"

using namespace std;

static const int ACCUMULATED_RENSA_SCORE[] = {
    1,
    40, 360, 1000, 2280, 4840, 
    8680, 13800, 20200, 27880, 36840,
    47080, 58600, 71400, 85480, 100840,
    117480, 135400, 154600, 175080, 
};

struct SortByFrames {
    bool operator()(const BasicRensaInfo& lhs, const BasicRensaInfo& rhs) const {
        if (lhs.frames != rhs.frames)
            return lhs.frames < rhs.frames;
        if (lhs.score != rhs.score)
            return lhs.score > rhs.score; // This '>' is intentional.
        return lhs.chains < rhs.chains;
    }
};

struct SortByInitiatingFrames {
    bool operator()(const FeasibleRensaInfo& lhs, const FeasibleRensaInfo& rhs) const {
        if (lhs.initiatingFrames != rhs.initiatingFrames)
            return lhs.initiatingFrames < rhs.initiatingFrames;

        // If initiatingFrames is the same, preferrable Rensa should come first.
        // High score is more preferrable, faster rensa is more preferrable.
        if (lhs.rensaInfo.score != rhs.rensaInfo.score)
            return lhs.rensaInfo.score > rhs.rensaInfo.score; 
        if (lhs.rensaInfo.frames != rhs.rensaInfo.frames)
            return lhs.rensaInfo.frames < rhs.rensaInfo.frames;

        return false;
    }
};

void EnemyInfo::updateFeasibleRensas(const Field& field, const vector<KumiPuyo>& kumiPuyos)
{
    m_feasibleRensaInfos.clear();

    vector<FeasibleRensaInfo> result;
    RensaDetector::findFeasibleRensas(result, field, 3, kumiPuyos); // TODO: What is 3?

    if (result.empty())
        return;

    sort(result.begin(), result.end(), SortByInitiatingFrames());
    m_feasibleRensaInfos.push_back(BasicRensaInfo(
                                       result.front().rensaInfo.chains,
                                       result.front().rensaInfo.score,
                                       result.front().initiatingFrames));

    for (vector<FeasibleRensaInfo>::iterator it = result.begin(); it != result.end(); ++it) {
        if (m_feasibleRensaInfos.back().score < it->rensaInfo.score) {
            DCHECK(m_feasibleRensaInfos.back().frames < it->initiatingFrames)
                << "feasible frames = " << m_feasibleRensaInfos.back().frames 
                << " initiating frames = " << it->initiatingFrames 
                << " score(1) = " << m_feasibleRensaInfos.back().score
                << " score(2) = " << it->rensaInfo.score << endl;
                
            m_feasibleRensaInfos.push_back(BasicRensaInfo(it->rensaInfo.chains, it->rensaInfo.score, it->initiatingFrames));
        }
    }    
}

void EnemyInfo::updatePossibleRensas(const Field& field, const vector<KumiPuyo>& kumiPuyos)
{
    PuyoSet kumipuyoSet;
    for (int i = 0; i < 3; ++i) {
        kumipuyoSet.add(kumiPuyos[i].axis, 1);
        kumipuyoSet.add(kumiPuyos[i].child, 1);
    }

    double averageHeight = 0;
    for (int x = 1; x <= Field::WIDTH; ++x)
        averageHeight += field.height(x) / 6.0;

    m_possibleRensaInfos.clear();

    vector<PossibleRensaInfo> result;
    result.reserve(100000);
    RensaDetector::findPossibleRensas(result, field, 3);

    vector<BasicRensaInfo> results;
    for (vector<PossibleRensaInfo>::iterator it = result.begin(); it != result.end(); ++it) {
        // おじゃまがあまり発生しそうにないのは無視
        if (it->rensaInfo.score < 70)
            continue;

        PuyoSet puyoSet(it->necessaryPuyoSet);
        puyoSet.sub(kumipuyoSet);

        // 見えているツモに加えて、さらに k ツモぐらい引くと 20% ぐらいの確率で引ける。
        int k = 0;
        for (k = 0; k < 15; ++k) {
            double p = TsumoPossibility::possibility(k * 2, puyoSet);
            if (p >= 0.2)
                break;
        }

        // 3 + k 回ほどぷよを引く必要があり、そのときに必要そうなフレーム数
        int frames = ((Field::HEIGHT - averageHeight) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI) * (3 + k);
        int score = it->rensaInfo.score;
        int chains = it->rensaInfo.chains;
        results.push_back(BasicRensaInfo(chains, score, frames));
    }

    if (results.empty())
        return;

    sort(results.begin(), results.end(), SortByFrames());    
    m_possibleRensaInfos.push_back(results.front());

    for (vector<BasicRensaInfo>::iterator it = results.begin(); it != results.end(); ++it) {
        if (m_possibleRensaInfos.back().score < it->score) {
            DCHECK(m_possibleRensaInfos.back().frames < it->frames);
            m_possibleRensaInfos.push_back(*it);
        }
    }
}

int EnemyInfo::estimateMaxScore(int frameId) const
{
    int scoreByFeasibleRensas = estimateMaxScoreFromFeasibleRensas(frameId);
    if (scoreByFeasibleRensas >= 0)
        return scoreByFeasibleRensas;

    int scoreByPossibleRensas = estimateMaxScoreFromPossibleRensas(frameId);
    if (scoreByPossibleRensas >= 0)
        return scoreByPossibleRensas;

    // これでもわからない場合、
    // その後、残ったフレーム数に対して、そのフレーム数で可能そうな連鎖を追加
    // すでにある連鎖に対しては、比率で倍率をあげる。
    // 7 -> 9 だったら、2 連鎖分の得点を追加し、7 連鎖で得られる得点に対して、
    // (9 連鎖で得られる得点)/(7 連鎖で得られる得点) を掛け算する。
    // TODO: Field の空きぐあいをどこかで見るべき
    int maxScore = -1;
    for (vector<BasicRensaInfo>::const_iterator it = possibleRensaInfos().begin(); it != possibleRensaInfos().end(); ++it) {
        int restFrames = frameId - it->frames - m_id;
        int numPossiblePuyos = 2 * (restFrames / (FRAMES_DROP_1_LINE * 10 + FRAMES_HORIZONTAL_MOVE + FRAMES_AFTER_NO_CHIGIRI));
        int newChains = min((numPossiblePuyos / 4) + it->chains, 19);
        double ratio = static_cast<double>(ACCUMULATED_RENSA_SCORE[newChains]) / ACCUMULATED_RENSA_SCORE[it->chains];
        int score = it->score * ratio - ACCUMULATED_RENSA_SCORE[it->chains];
        maxScore = std::max(maxScore, score);
    }

    if (maxScore >= 0)
        return maxScore;

    int restFrames = frameId - m_id;
    int numPossiblePuyos = 2 * (restFrames / (FRAMES_DROP_1_LINE * 10 + FRAMES_HORIZONTAL_MOVE + FRAMES_AFTER_NO_CHIGIRI));
    int newChains = min((numPossiblePuyos / 4), 19);
    return ACCUMULATED_RENSA_SCORE[newChains];
}

int EnemyInfo::estimateMaxScoreFromFeasibleRensas(int frameId) const
{
    return estimateMaxScoreFrom(frameId, m_feasibleRensaInfos);
}

int EnemyInfo::estimateMaxScoreFromPossibleRensas(int frameId) const
{
    return estimateMaxScoreFrom(frameId, m_possibleRensaInfos);
}

int EnemyInfo::estimateMaxScoreFrom(int frameId, const vector<BasicRensaInfo>& rensaInfos) const
{
    if (rensaInfos.empty())
        return -1;

    if (rensaInfos.back().frames + m_id < frameId)
        return -1;

    for (vector<BasicRensaInfo>::const_iterator it = rensaInfos.begin(); it != rensaInfos.end(); ++it) {
        if (it->frames + m_id <= frameId)
            return it->score;
    }

    return -1;
}
