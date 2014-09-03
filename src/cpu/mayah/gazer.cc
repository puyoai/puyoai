#include "gazer.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include <glog/logging.h>

#include "core/constant.h"
#include "core/kumipuyo.h"
#include "core/algorithm/rensa_detector.h"
#include "core/algorithm/puyo_possibility.h"

using namespace std;

static const int ACCUMULATED_RENSA_SCORE[] = {
    1,
    40, 360, 1000, 2280, 4840,
    8680, 13800, 20200, 27880, 36840,
    47080, 58600, 71400, 85480, 100840,
    117480, 135400, 154600, 175080,
};

struct SortByFrames {
    bool operator()(const EstimatedRensaInfo& lhs, const EstimatedRensaInfo& rhs) const {
        if (lhs.initiatingFrames != rhs.initiatingFrames)
            return lhs.initiatingFrames < rhs.initiatingFrames;

        if (lhs.score != rhs.score)
            return lhs.score > rhs.score; // This '>' is intentional.

        return lhs.chains > rhs.chains; // This '>' is intentional.
    }
};

struct SortByInitiatingFrames {
    bool operator()(const FeasibleRensaInfo& lhs, const FeasibleRensaInfo& rhs) const {
        if (lhs.initiatingFrames() != rhs.initiatingFrames())
            return lhs.initiatingFrames() < rhs.initiatingFrames();

        // If initiatingFrames is the same, preferrable Rensa should come first.
        // High score is more preferrable, faster rensa is more preferrable.
        if (lhs.score() != rhs.score())
            return lhs.score() > rhs.score();
        if (lhs.totalFrames() != rhs.totalFrames())
            return lhs.totalFrames() < rhs.totalFrames();

        return false;
    }
};

void Gazer::updateFeasibleRensas(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    m_feasibleRensaInfos.clear();

    vector<FeasibleRensaInfo> result = RensaDetector::findFeasibleRensas(field, kumipuyoSeq);

    if (result.empty())
        return;

    sort(result.begin(), result.end(), SortByInitiatingFrames());
    m_feasibleRensaInfos.push_back(EstimatedRensaInfo(
                                       result.front().chains(),
                                       result.front().score(),
                                       result.front().initiatingFrames()));

    for (vector<FeasibleRensaInfo>::iterator it = result.begin(); it != result.end(); ++it) {
        if (m_feasibleRensaInfos.back().score < it->score()) {
            DCHECK(m_feasibleRensaInfos.back().initiatingFrames < it->initiatingFrames())
                << "feasible frames = " << m_feasibleRensaInfos.back().initiatingFrames
                << " initiating frames = " << it->initiatingFrames()
                << " score(1) = " << m_feasibleRensaInfos.back().score
                << " score(2) = " << it->score() << endl;

            m_feasibleRensaInfos.push_back(EstimatedRensaInfo(
                                               it->chains(),
                                               it->score(),
                                               it->initiatingFrames()));

        }
    }
}

void Gazer::updatePossibleRensas(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    PuyoSet kumipuyoSet;
    for (const auto& x : kumipuyoSeq.underlyingData()) {
        kumipuyoSet.add(x.axis);
        kumipuyoSet.add(x.child);
    }

    double averageHeight = 0;
    for (int x = 1; x <= CoreField::WIDTH; ++x)
        averageHeight += field.height(x) / 6.0;

    m_possibleRensaInfos.clear();

    vector<EstimatedRensaInfo> results;
    results.reserve(1000);
    auto callback = [&](const CoreField&, const RensaResult& rensaResult,
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos) {
        // おじゃまがあまり発生しそうにないのは無視
        if (rensaResult.score < 70)
            return;

        PuyoSet puyoSet;
        puyoSet.add(keyPuyos);
        puyoSet.add(firePuyos);
        puyoSet.sub(kumipuyoSet);

        // 見えているツモに加えて、さらに k ツモぐらい引くと 20% ぐらいの確率で引ける。
        int k = 0;
        for (k = 0; k < 15; ++k) {
            double p = TsumoPossibility::possibility(k * 2, puyoSet);
            if (p >= 0.2)
                break;
        }

        // 3 + k 回ほどぷよを引く必要があり、そのときに必要そうなフレーム数
        int initiatingFrames = (FRAMES_TO_DROP_FAST[static_cast<int>(std::ceil(CoreField::HEIGHT - averageHeight))] +
                                FRAMES_GROUNDING + FRAMES_PREPARING_NEXT) * (3 + k);
        int score = rensaResult.score;
        int chains = rensaResult.chains;
        results.push_back(EstimatedRensaInfo(chains, score, initiatingFrames));
    };

    RensaDetector::iteratePossibleRensas(field, 3, callback);
    if (results.empty())
        return;

    sort(results.begin(), results.end(), SortByFrames());
    m_possibleRensaInfos.push_back(results.front());

    for (auto it = results.begin(); it != results.end(); ++it) {
        if (m_possibleRensaInfos.back().score < it->score) {
            DCHECK(m_possibleRensaInfos.back().initiatingFrames < it->initiatingFrames);
            m_possibleRensaInfos.push_back(*it);
        }
    }
}

int Gazer::estimateMaxScore(int frameId) const
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
    for (auto it = possibleRensaInfos().begin(); it != possibleRensaInfos().end(); ++it) {
        int restFrames = frameId - (it->initiatingFrames + m_id);
        int numPossiblePuyos = 2 * (restFrames / (FRAMES_TO_DROP_FAST[10] + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT));
        int newAdditionalChains = min(numPossiblePuyos / 4, 19);

        // TODO(mayah): newChains should not be negative. restFrames is negative?
        if (newAdditionalChains < 0)
            newAdditionalChains = 0;

        int newTotalChains = std::max(std::min(it->chains + newAdditionalChains, 19), 0);
        double ratio = static_cast<double>(ACCUMULATED_RENSA_SCORE[newTotalChains]) / ACCUMULATED_RENSA_SCORE[it->chains];
        int score = it->score * ratio - ACCUMULATED_RENSA_SCORE[newAdditionalChains];
        maxScore = std::max(maxScore, score);
    }

    if (maxScore >= 0)
        return maxScore;

    // When there is not possible rensa.
    int restFrames = frameId - m_id;
    int numPossiblePuyos = 2 * (restFrames / (FRAMES_TO_DROP_FAST[10] + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING));
    int newChains = min((numPossiblePuyos / 4), 19);
    // TODO(mayah): newChains should not be negative. restFrames is negative?
    if (newChains < 0)
        newChains = 0;
    return ACCUMULATED_RENSA_SCORE[newChains];
}

int Gazer::estimateMaxScoreFromFeasibleRensas(int frameId) const
{
    return estimateMaxScoreFrom(frameId, m_feasibleRensaInfos);
}

int Gazer::estimateMaxScoreFromPossibleRensas(int frameId) const
{
    return estimateMaxScoreFrom(frameId, m_possibleRensaInfos);
}

int Gazer::estimateMaxScoreFrom(int frameId, const vector<EstimatedRensaInfo>& rensaInfos) const
{
    if (rensaInfos.empty())
        return -1;

    if (rensaInfos.back().initiatingFrames + m_id < frameId)
        return -1;

    for (auto it = rensaInfos.begin(); it != rensaInfos.end(); ++it) {
        if (frameId <= it->initiatingFrames + m_id)
            return it->score;
    }

    return -1;
}
