#include "gazer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include <glog/logging.h>

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/algorithm/rensa_detector.h"
#include "core/constant.h"
#include "core/field_bit_field.h"
#include "core/kumipuyo_seq.h"
#include "core/score.h"

using namespace std;

static const int ACCUMULATED_RENSA_SCORE[] = {
    1,
    40, 360, 1000, 2280, 4840,
    8680, 13800, 20200, 27880, 36840,
    47080, 58600, 71400, 85480, 100840,
    117480, 135400, 154600, 175080,
};

struct SortByFrames {
    bool operator()(const EstimatedRensaInfo& lhs, const EstimatedRensaInfo& rhs) const
    {
        if (lhs.framesToIgnite != rhs.framesToIgnite)
            return lhs.framesToIgnite < rhs.framesToIgnite;

        // The rest of '>' is intentional.
        if (lhs.score != rhs.score)
            return lhs.score > rhs.score;
        if (lhs.chains != rhs.chains)
            return lhs.score > rhs.chains;
        return lhs.coefResult.coef(lhs.chains) > rhs.coefResult.coef(rhs.chains);
    }
};

std::string EstimatedRensaInfo::toString() const
{
    char buf[80];
    sprintf(buf, "frames, chains, score = %d, %d, %d", framesToIgnite, chains, score);
    return buf;
}

int GazeResult::estimateMaxScore(int frameId, const PlayerState& enemy) const
{
    CHECK_LE(frameIdGazedAt_, frameId)
        << "Gazer is requested to check the past frame estimated score."
        << " frameId=" << frameId
        << " frameIdGazedAt=" << frameIdGazedAt_;

    if (enemy.isRensaOngoing && frameId <= enemy.finishingRensaFrameId) {
        return enemy.ongoingRensaResult.score;
    }

    int scoreByFeasibleRensas = estimateMaxScoreFromFeasibleRensas(frameId);
    if (scoreByFeasibleRensas >= 0)
        return scoreByFeasibleRensas;

    int scoreByPossibleRensas = estimateMaxScoreFromPossibleRensas(frameId);
    if (scoreByPossibleRensas >= 0)
        return scoreByPossibleRensas;

    // We cannot estimate the score using feasible rensas and possible rensas.

    int maxScore = -1;
    for (auto it = possibleRensaInfos_.begin(); it != possibleRensaInfos_.end(); ++it) {
        int restFrames = frameId - (it->framesToIgnite + frameIdGazedAt());
        int numPossiblePuyos = 2 * (restFrames / (FRAMES_TO_DROP_FAST[10] + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT));
        // At max, enemy will be able ot puyo restEmptyField. We have counted the puyos for possibleRensaInfos,
        // we substract 6 from restEmptyField_.
        numPossiblePuyos = max(0, min(restEmptyField_ - 6, numPossiblePuyos));
        int newAdditionalChains = min(numPossiblePuyos / 4, 19);

        // TODO(mayah): newChains should not be negative. restFrames is negative?
        if (newAdditionalChains < 0)
            newAdditionalChains = 0;
        if (newAdditionalChains + it->chains > 19)
            newAdditionalChains = 19 - it->chains;

        int rensaCoef[20] {};
        int rensaNumErased[20] {};
        for (int i = 1; i <= newAdditionalChains; ++i) {
            rensaCoef[i] = chainBonus(i);
            if (rensaCoef[i] == 0)
                rensaCoef[i] = 1;
            rensaNumErased[i] = 4;
        }
        for (int i = 1; i <= it->chains; ++i) {
            int j = i + newAdditionalChains;
            rensaCoef[j] = it->coefResult.coef(i) - chainBonus(i) + chainBonus(j);
            rensaNumErased[j] = it->coefResult.numErased(i);
        }
        int score = 0;
        for (int i = 1; i <= newAdditionalChains + it->chains; ++i)
            score += rensaCoef[i] * rensaNumErased[i] * 10;

        maxScore = std::max(maxScore, score);
    }

    if (maxScore >= 0)
        return maxScore;

    // When there is not possible rensa.
    int restFrames = frameId - frameIdGazedAt();
    int numPossiblePuyos = 2 * (restFrames / (FRAMES_TO_DROP_FAST[10] + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING));
    numPossiblePuyos = max(0, min(restEmptyField_ - 6, numPossiblePuyos));
    int newChains = min((numPossiblePuyos / 4), 19);
    // TODO(mayah): newChains should not be negative. restFrames is negative?
    if (newChains < 0)
        newChains = 0;
    return ACCUMULATED_RENSA_SCORE[newChains];
}

int GazeResult::estimateMaxScoreFromFeasibleRensas(int frameId) const
{
    return estimateMaxScoreFrom(frameId, feasibleRensaInfos_);
}

int GazeResult::estimateMaxScoreFromPossibleRensas(int frameId) const
{
    return estimateMaxScoreFrom(frameId, possibleRensaInfos_);
}

int GazeResult::estimateMaxScoreFrom(int frameId, const vector<EstimatedRensaInfo>& rensaInfos) const
{
    if (rensaInfos.empty())
        return -1;

    if (rensaInfos.back().framesToIgnite + frameIdGazedAt() < frameId)
        return -1;

    for (auto it = rensaInfos.begin(); it != rensaInfos.end(); ++it) {
        if (frameId <= it->framesToIgnite + frameIdGazedAt())
            return it->score;
    }

    return -1;
}

string GazeResult::toRensaInfoString() const
{
    stringstream ss;
    ss << "gazed at frameId: " << frameIdGazedAt_ << endl;
    ss << "Possible rensa infos: " << endl;
    for (const auto& info : possibleRensaInfos_)
        ss << info.toString() << endl;
    ss << "Feasible rensa infos: " << endl;
    for (const auto& info : feasibleRensaInfos_)
        ss << info.toString() << endl;

    return ss.str();
}

// ----------------------------------------------------------------------

void Gazer::initialize(int frameIdGameWillBegin)
{
    frameIdGazedAt_ = frameIdGameWillBegin;
    restEmptyField_ = 72;
    feasibleRensaInfos_.clear();
    possibleRensaInfos_.clear();
}

void Gazer::gaze(int frameId, const CoreField& cf, const KumipuyoSeq& seq)
{
    setFrameIdGazedAt(frameId);
    updateFeasibleRensas(cf, seq);
    updatePossibleRensas(cf, seq);

    FieldBitField checked;
    restEmptyField_ = cf.countConnectedPuyos(3, 12, &checked);
}

void Gazer::updateFeasibleRensas(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    feasibleRensaInfos_.clear();

    // It might take long time if size() >= 4. Consider only size <= 3.
    KumipuyoSeq seq(kumipuyoSeq);
    if (seq.size() >= 4)
        seq = seq.subsequence(0, 3);

    std::vector<EstimatedRensaInfo> results;
    auto f = [&results](const CoreField& cf, const std::vector<Decision>& /*decisions*/,
                        int /*numChigiri*/, int framesToIgnite, int /*lastDropFrames*/, bool shouldFire) {
        if (!shouldFire)
            return;

        CoreField copied(cf);
        RensaCoefResult coefResult;
        RensaResult rensaResult = copied.simulate(&coefResult);
        results.emplace_back(rensaResult.chains, rensaResult.score, framesToIgnite, coefResult);
    };
    Plan::iterateAvailablePlansWithoutFiring(field, seq, seq.size(), f);

    if (results.empty())
        return;

    sort(results.begin(), results.end(), SortByFrames());
    feasibleRensaInfos_.push_back(results.front());

    for (const auto& info : results) {
        if (info.score <= feasibleRensaInfos_.back().score)
            continue;

        DCHECK(feasibleRensaInfos_.back().framesToIgnite < info.framesToIgnite)
            << "feasible frames = " << feasibleRensaInfos_.back().framesToIgnite
            << " initiating frames = " << info.framesToIgnite
            << " score(1) = " << feasibleRensaInfos_.back().score
            << " score(2) = " << info.score << endl;

        feasibleRensaInfos_.push_back(info);
    }
}

void Gazer::updatePossibleRensas(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    possibleRensaInfos_.clear();

    double averageHeight = 0;
    for (int x = 1; x <= CoreField::WIDTH; ++x)
        averageHeight += field.height(x) / 6.0;

    vector<EstimatedRensaInfo> results;
    results.reserve(1000);
    auto callback = [&](const CoreField&, const RensaResult& rensaResult,
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                        const RensaCoefResult& coefResult) {
        // Ignore rensa whose power is really small.
        if (rensaResult.score < 70)
            return;

        PuyoSet puyoSet;
        puyoSet.add(keyPuyos);
        puyoSet.add(firePuyos);

        int necessaryHands = kumipuyoSeq.size();
        for (int i = 0; i < kumipuyoSeq.size(); ++i) {
            puyoSet.sub(kumipuyoSeq.axis(i));
            puyoSet.sub(kumipuyoSeq.child(i));
            if (puyoSet.isEmpty()) {
                necessaryHands = i + 1;
                break;
            }
        }

        // When the enemy took |k| hands, enemy will be able to fire the rensa in 30%.
        if (!puyoSet.isEmpty()) {
            necessaryHands += (TsumoPossibility::necessaryPuyos(puyoSet, 0.3) + 1) / 2;
        }

        // We need to remove last hand frames.
        if (necessaryHands > 1)
            --necessaryHands;

        // Estimate the number of frames to initiate the rensa.
        int heightMove = std::max(0, static_cast<int>(std::ceil(CoreField::HEIGHT - averageHeight)));
        int framesToIgnite = (FRAMES_TO_DROP_FAST[heightMove] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT) * necessaryHands;
        int score = rensaResult.score;
        int chains = rensaResult.chains;
        results.emplace_back(chains, score, framesToIgnite, coefResult);
    };

    RensaDetector::iteratePossibleRensasWithCoefTracking(field, 3,
                                                         RensaDetectorStrategy::defaultFloatStrategy(),
                                                         callback);
    if (results.empty())
        return;

    sort(results.begin(), results.end(), SortByFrames());
    possibleRensaInfos_.push_back(results.front());

    for (const auto& info : results) {
        if (info.score <= possibleRensaInfos_.back().score)
            continue;

        DCHECK(possibleRensaInfos_.back().framesToIgnite < info.framesToIgnite);
        possibleRensaInfos_.push_back(info);
    }
}

GazeResult Gazer::gazeResult() const
{
    return GazeResult(frameIdGazedAt_, restEmptyField_, feasibleRensaInfos_, possibleRensaInfos_);
}
