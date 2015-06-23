#include "gazer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include <glog/logging.h>

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/algorithm/rensa_detector.h"
#include "core/field_checker.h"
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
        if (lhs.framesToIgnite() != rhs.framesToIgnite())
            return lhs.framesToIgnite() < rhs.framesToIgnite();

        // The rest of '>' is intentional.
        if (lhs.score() != rhs.score())
            return lhs.score() > rhs.score();
        if (lhs.chains() != rhs.chains())
            return lhs.score() > rhs.chains();

        if (lhs.rensaFrames() != rhs.rensaFrames())
            return lhs.rensaFrames() < rhs.rensaFrames();

        return lhs.coefResult.coef(lhs.chains()) > rhs.coefResult.coef(rhs.chains());
    }
};

std::string EstimatedRensaInfo::toString() const
{
    char buf[80];
    sprintf(buf, "framesToIgnite, chains, score, totalFrames = %d, %d, %d, %d", framesToIgnite(), chains(), score(), totalFrames());
    return buf;
}

void GazeResult::reset(int frameId, int numReachableSpaces)
{
    frameIdGazedAt_ = frameId;
    numReachableSpaces_ = numReachableSpaces;
    feasibleRensaInfos_.clear();
    possibleRensaInfos_.clear();
}

int GazeResult::estimateMaxScore(int frameId, const PlayerState& enemy) const
{
    CHECK_LE(frameIdGazedAt_, frameId)
        << "Gazer is requested to check the past frame estimated score."
        << " frameId=" << frameId
        << " frameIdGazedAt=" << frameIdGazedAt_;

    // TODO(mayah): How to handle this?
    if (enemy.isRensaOngoing() && frameId <= enemy.rensaFinishingFrameId()) {
        return enemy.currentRensaResult.score;
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
        int restFrames = frameId - (it->framesToIgnite() + frameIdGazedAt());
        int numPossiblePuyos = 2 * (restFrames / (FRAMES_TO_DROP_FAST[10] + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT));
        // At max, enemy will be able ot puyo restEmptyField. We have counted the puyos for possibleRensaInfos,
        // we substract 6 from restEmptyField_.
        numPossiblePuyos = max(0, min(numReachableSpaces_ - 6, numPossiblePuyos));
        int newAdditionalChains = min(numPossiblePuyos / 4, 19);

        // TODO(mayah): newChains should not be negative. restFrames is negative?
        if (newAdditionalChains < 0)
            newAdditionalChains = 0;
        if (newAdditionalChains + it->chains() > 19)
            newAdditionalChains = 19 - it->chains();

        maxScore = std::max(maxScore, it->coefResult.score(newAdditionalChains));
    }

    if (maxScore >= 0)
        return maxScore;

    // When there is not possible rensa.
    int restFrames = frameId - frameIdGazedAt();
    int numPossiblePuyos = 2 * (restFrames / (FRAMES_TO_DROP_FAST[10] + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING));
    numPossiblePuyos = max(0, min(numReachableSpaces_ - 6, numPossiblePuyos));
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

    if (rensaInfos.back().framesToIgnite() + frameIdGazedAt() < frameId)
        return -1;

    for (auto it = rensaInfos.begin(); it != rensaInfos.end(); ++it) {
        if (frameId <= it->framesToIgnite() + frameIdGazedAt())
            return it->score();
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
    gazeResult_.reset(frameIdGameWillBegin, 72);
}

void Gazer::gaze(int frameId, const CoreField& cf, const KumipuyoSeq& seq)
{
    int numReachableSpaces = cf.countConnectedPuyos(3, 12);
    gazeResult_.reset(frameId, numReachableSpaces);

    updateFeasibleRensas(cf, seq);
    updatePossibleRensas(cf, seq);
}

void Gazer::updateFeasibleRensas(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    // It might take long time if size() >= 4. Consider only size <= 3.
    KumipuyoSeq seq(kumipuyoSeq);
    if (seq.size() >= 4)
        seq = seq.subsequence(0, 3);

    std::vector<EstimatedRensaInfo> results;
    {
        auto f = [&results](const CoreField& cf, const std::vector<Decision>& /*decisions*/,
                            int /*numChigiri*/, int framesToIgnite, int /*lastDropFrames*/, bool shouldFire) {
            if (!shouldFire)
                return;

            CoreField copied(cf);
            RensaCoefTracker tracker;
            RensaResult rensaResult = copied.simulate(&tracker);
            results.emplace_back(IgnitionRensaResult(rensaResult, framesToIgnite), tracker.result());
        };
        Plan::iterateAvailablePlansWithoutFiring(field, seq, seq.size(), f);

        if (results.empty())
            return;

        sort(results.begin(), results.end(), SortByFrames());
    }

    std::vector<EstimatedRensaInfo> feasibleRensaInfos;

    feasibleRensaInfos.push_back(results.front());

    for (const auto& info : results) {
        if (info.score() <= feasibleRensaInfos.back().score())
            continue;

        DCHECK(feasibleRensaInfos.back().framesToIgnite() < info.framesToIgnite())
            << "feasible frames = " << feasibleRensaInfos.back().framesToIgnite()
            << " initiating frames = " << info.framesToIgnite()
            << " score(1) = " << feasibleRensaInfos.back().score()
            << " score(2) = " << info.score() << endl;

        feasibleRensaInfos.push_back(info);
    }

    gazeResult_.setFeasibleRensaInfo(std::move(feasibleRensaInfos));
}

void Gazer::updatePossibleRensas(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    double averageHeight = 0;
    for (int x = 1; x <= FieldConstant::WIDTH; ++x)
        averageHeight += field.height(x) / 6.0;

    vector<EstimatedRensaInfo> results;
    {
        results.reserve(20000);
        auto callback = [&](CoreField&& cf, const ColumnPuyoList& puyosToComplement) -> RensaResult {
            RensaCoefTracker tracker;
            RensaResult rensaResult = cf.simulate(&tracker);

            // Ignore rensa whose power is really small.
            if (rensaResult.score < 70)
                return rensaResult;

            PuyoSet puyoSet;
            puyoSet.add(puyosToComplement);
            int necessaryPuyos = PuyoPossibility::necessaryPuyos(puyoSet, kumipuyoSeq, 0.3);
            int necessaryHands = (necessaryPuyos + 1) / 2;

            // We need to remove last hand frames, since we'd like to calculate framesToIgnite.
            if (necessaryHands > 1)
                --necessaryHands;

            // Estimate the number of frames to initiate the rensa.
            int heightMove = std::max(0, static_cast<int>(std::ceil(FieldConstant::HEIGHT - averageHeight)));
            int framesToIgnite = (FRAMES_TO_DROP_FAST[heightMove] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT) * necessaryHands;
            results.emplace_back(IgnitionRensaResult(rensaResult, framesToIgnite), tracker.result());

            return rensaResult;
        };

        RensaDetector::detectIteratively(field, RensaDetectorStrategy::defaultFloatStrategy(), 3, callback);
        if (results.empty())
            return;

        sort(results.begin(), results.end(), SortByFrames());
    }

    vector<EstimatedRensaInfo> possibleRensaInfos;
    possibleRensaInfos.push_back(results.front());

    for (const auto& info : results) {
        if (info.score() <= possibleRensaInfos.back().score())
            continue;

        DCHECK(possibleRensaInfos.back().framesToIgnite() < info.framesToIgnite());
        possibleRensaInfos.push_back(info);
    }

    gazeResult_.setPossibleRensaInfo(std::move(possibleRensaInfos));
}
