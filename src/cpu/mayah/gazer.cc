#include "gazer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include <glog/logging.h>

#include "core/constant.h"
#include "core/field/field_bit_field.h"
#include "core/kumipuyo_seq.h"
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
    bool operator()(const EstimatedRensaInfo& lhs, const EstimatedRensaInfo& rhs) const
    {
        if (lhs.framesToInitiate != rhs.framesToInitiate)
            return lhs.framesToInitiate < rhs.framesToInitiate;

        if (lhs.score != rhs.score)
            return lhs.score > rhs.score; // This '>' is intentional.

        return lhs.chains > rhs.chains; // This '>' is intentional.
    }
};

struct SortByInitiatingFrames {
    bool operator()(const FeasibleRensaInfo& lhs, const FeasibleRensaInfo& rhs) const
    {
        if (lhs.framesToInitiate() != rhs.framesToInitiate())
            return lhs.framesToInitiate() < rhs.framesToInitiate();

        // If framesToInitiate is the same, preferrable Rensa should come first.
        // High score is more preferrable, faster rensa is more preferrable.
        if (lhs.score() != rhs.score())
            return lhs.score() > rhs.score();
        if (lhs.totalFrames() != rhs.totalFrames())
            return lhs.totalFrames() < rhs.totalFrames();

        return false;
    }
};

void Gazer::initialize(int frameIdGameWillBegin)
{
    frameIdGazedAt_ = frameIdGameWillBegin;
    restEmptyField_ = 72;
    additionalThoughtInfo_ = AdditionalThoughtInfo();
    feasibleRensaInfos_.clear();
    possibleRensaInfos_.clear();
}

void Gazer::gaze(int frameId, const CoreField& cf, const KumipuyoSeq& seq)
{
    KumipuyoSeq s = (seq.size() > 4 ? seq.subsequence(0, 3) : seq);
    setFrameIdGazedAt(frameId);
    updateFeasibleRensas(cf, s);
    updatePossibleRensas(cf, s);

    FieldBitField checked;
    restEmptyField_ = cf.countConnectedPuyos(3, 12, &checked);
}

void Gazer::updateFeasibleRensas(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    feasibleRensaInfos_.clear();

    vector<FeasibleRensaInfo> result = RensaDetector::findFeasibleRensas(field, kumipuyoSeq);
    if (result.empty())
        return;

    sort(result.begin(), result.end(), SortByInitiatingFrames());
    feasibleRensaInfos_.push_back(EstimatedRensaInfo(
                                      result.front().chains(),
                                      result.front().score(),
                                      result.front().framesToInitiate()));

    for (const auto& info : result) {
        if (info.score() <= feasibleRensaInfos_.back().score)
            continue;

        DCHECK(feasibleRensaInfos_.back().framesToInitiate < info.framesToInitiate())
            << "feasible frames = " << feasibleRensaInfos_.back().framesToInitiate
            << " initiating frames = " << info.framesToInitiate()
            << " score(1) = " << feasibleRensaInfos_.back().score
            << " score(2) = " << info.score() << endl;

        feasibleRensaInfos_.push_back(EstimatedRensaInfo(info.chains(), info.score(), info.framesToInitiate()));
    }
}

void Gazer::updatePossibleRensas(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    possibleRensaInfos_.clear();

    PuyoSet kumipuyoSet;
    for (const auto& x : kumipuyoSeq) {
        kumipuyoSet.add(x.axis);
        kumipuyoSet.add(x.child);
    }

    double averageHeight = 0;
    for (int x = 1; x <= CoreField::WIDTH; ++x)
        averageHeight += field.height(x) / 6.0;

    vector<EstimatedRensaInfo> results;
    results.reserve(1000);
    auto callback = [&](const CoreField&, const RensaResult& rensaResult,
                        const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos) {
        // Ignore rensa whose power is really small.
        if (rensaResult.score < 70)
            return;

        PuyoSet puyoSet;
        puyoSet.add(keyPuyos);
        puyoSet.add(firePuyos);
        puyoSet.sub(kumipuyoSet);

        // When the enemy took |k| hands, enemy will be able to fire the rensa in 20%.
        int k = 0;
        for (k = 0; k < 15; ++k) {
            double p = TsumoPossibility::possibility(k * 2, puyoSet);
            if (p >= 0.2)
                break;
        }

        // Estimate the number of frames to initiate the rensa.
        int heightMove = std::max(0, static_cast<int>(std::ceil(CoreField::HEIGHT - averageHeight)));
        int framesToInitiate = (FRAMES_TO_DROP_FAST[heightMove] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT) * (3 + k);
        int score = rensaResult.score;
        int chains = rensaResult.chains;
        results.push_back(EstimatedRensaInfo(chains, score, framesToInitiate));
    };

    RensaDetector::iteratePossibleRensas(field, 3, callback, RensaDetector::Mode::FLOAT);
    if (results.empty())
        return;

    sort(results.begin(), results.end(), SortByFrames());
    possibleRensaInfos_.push_back(results.front());

    for (const auto& info : results) {
        if (info.score <= possibleRensaInfos_.back().score)
            continue;

        DCHECK(possibleRensaInfos_.back().framesToInitiate < info.framesToInitiate);
        possibleRensaInfos_.push_back(info);
    }
}

int Gazer::estimateMaxScore(int frameId) const
{
    if (frameId < frameIdGazedAt_) {
        CHECK(false) << "Gazer is requested to check the past frame estimated score."
                     << " frameId=" << frameId
                     << " frameIdGazedAt=" << frameIdGazedAt_;
        return -1;
    }

    if (isRensaOngoing() && frameId <= ongoingRensaFinishingFrameId()) {
        return ongoingRensaResult().score;
    }

    int scoreByFeasibleRensas = estimateMaxScoreFromFeasibleRensas(frameId);
    if (scoreByFeasibleRensas >= 0)
        return scoreByFeasibleRensas;

    int scoreByPossibleRensas = estimateMaxScoreFromPossibleRensas(frameId);
    if (scoreByPossibleRensas >= 0)
        return scoreByPossibleRensas;

    // We cannot estimate the score using feasible rensas and possible rensas.

    int maxScore = -1;
    for (auto it = possibleRensaInfos().begin(); it != possibleRensaInfos().end(); ++it) {
        int restFrames = frameId - (it->framesToInitiate + frameIdGazedAt());
        int numPossiblePuyos = 2 * (restFrames / (FRAMES_TO_DROP_FAST[10] + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT));
        // At max, enemy will be able ot puyo restEmptyField. We have counted the puyos for possibleRensaInfos,
        // we substract 6 from restEmptyField_.
        numPossiblePuyos = max(0, min(restEmptyField_ - 6, numPossiblePuyos));
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
    int restFrames = frameId - frameIdGazedAt();
    int numPossiblePuyos = 2 * (restFrames / (FRAMES_TO_DROP_FAST[10] + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING));
    numPossiblePuyos = max(0, min(restEmptyField_ - 6, numPossiblePuyos));
    int newChains = min((numPossiblePuyos / 4), 19);
    // TODO(mayah): newChains should not be negative. restFrames is negative?
    if (newChains < 0)
        newChains = 0;
    return ACCUMULATED_RENSA_SCORE[newChains];
}

int Gazer::estimateMaxScoreFromFeasibleRensas(int frameId) const
{
    return estimateMaxScoreFrom(frameId, feasibleRensaInfos_);
}

int Gazer::estimateMaxScoreFromPossibleRensas(int frameId) const
{
    return estimateMaxScoreFrom(frameId, possibleRensaInfos_);
}

int Gazer::estimateMaxScoreFrom(int frameId, const vector<EstimatedRensaInfo>& rensaInfos) const
{
    if (rensaInfos.empty())
        return -1;

    if (rensaInfos.back().framesToInitiate + frameIdGazedAt() < frameId)
        return -1;

    for (auto it = rensaInfos.begin(); it != rensaInfos.end(); ++it) {
        if (frameId <= it->framesToInitiate + frameIdGazedAt())
            return it->score;
    }

    return -1;
}

string Gazer::toRensaInfoString() const
{
    stringstream ss;
    ss << "gazed at frameId: " << frameIdGazedAt_ << endl;
    ss << "Possible rensa infos: " << endl;
    for (const auto& info : possibleRensaInfos())
        ss << info.toString() << endl;
    ss << "Feasible rensa infos: " << endl;
    for (const auto& info : feasibleRensaInfos())
        ss << info.toString() << endl;

    return ss.str();
}
