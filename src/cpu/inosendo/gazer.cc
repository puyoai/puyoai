#include "gazer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include <glog/logging.h>

#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/field_checker.h"
#include "core/kumipuyo_seq.h"
#include "core/probability/puyo_set_probability.h"
#include "core/score.h"

using namespace std;

struct SortByFrames {
    bool operator()(const RensaHand& lhs, const RensaHand& rhs) const
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

void GazeResult::reset(int frameIdToStartNextMove, int numReachableSpaces)
{
    frameIdToStartNextMove_ = frameIdToStartNextMove;
    numReachableSpaces_ = numReachableSpaces;
}

int GazeResult::estimateMaxScore(int frameId, const PlayerState& enemy) const
{
    // TODO(mayah): How to handle this?
    if (enemy.isRensaOngoing() && frameId <= enemy.rensaFinishingFrameId()) {
        return enemy.currentRensaResult.score;
    }

    // We need to check this after checking enemy.isRensaOngoing().
    // Since gaze frameId will be the time just after the rensa is finished.
    CHECK_LE(frameIdToStartNextMove(), frameId)
        << "Gazer is requested to check the past frame estimated score."
        << " frameId=" << frameId
        << " frameIdToStartNextMove=" << frameIdToStartNextMove();

    int scoreByFeasibleRensas = estimateMaxScoreFromFeasibleRensas(frameId);
    if (scoreByFeasibleRensas >= 0)
        return scoreByFeasibleRensas;

    int scoreByPossibleRensas = estimateMaxScoreFromPossibleRensas(frameId);
    if (scoreByPossibleRensas >= 0)
        return scoreByPossibleRensas;

    return 0;
}

int GazeResult::estimateMaxScoreFromFeasibleRensas(int frameId) const
{
    if (feasibleRensaHandTree_.nodes().empty())
        return -1;

    int maxScore = -1;
    const RensaHandNode& node = feasibleRensaHandTree_.node(0);
    for (const auto& edge : node.edges()) {
        if (frameId <= frameIdToStartNextMove() + edge.rensaHand().framesToIgnite()) {
            maxScore = std::max(maxScore, edge.rensaHand().score());
        }
    }

    return maxScore;
}

int GazeResult::estimateMaxScoreFromPossibleRensas(int frameId) const
{
    if (possibleRensaHandTree_.nodes().empty())
        return -1;

    int maxScore = -1;
    const RensaHandNode& node = possibleRensaHandTree_.node(0);
    for (const auto& edge : node.edges()) {
        int restFrames = frameId - frameIdToStartNextMove() + edge.rensaHand().framesToIgnite();
        if (restFrames < 0)
            continue;

        // Fire immediately?
        maxScore = std::max(maxScore, edge.rensaHand().score());

        // Fire by making this large?
        int numPossiblePuyos = 2 * (restFrames / (FRAMES_TO_DROP_FAST[10] + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT));
        // At max, enemy will be able ot puyo restEmptyField. We have counted the puyos for possibleRensaInfos,
        // we substract 6 from restEmptyField_.
        numPossiblePuyos = max(0, min(numReachableSpaces_ - 6, numPossiblePuyos));
        int newAdditionalChains = min(numPossiblePuyos / 4, 19);
        // TODO(mayah): newChains should not be negative. restFrames is negative?
        if (newAdditionalChains < 0)
            newAdditionalChains = 0;
        if (newAdditionalChains + edge.rensaHand().chains() > 19)
            newAdditionalChains = 19 - edge.rensaHand().chains();

        maxScore = std::max(maxScore, edge.rensaHand().coefResult.score(newAdditionalChains));
    }

    {
        // When there is not possible rensa.
        int restFrames = frameId - frameIdToStartNextMove();
        int numPossiblePuyos = 2 * (restFrames / (FRAMES_TO_DROP_FAST[10] + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING));
        numPossiblePuyos = max(0, min(numReachableSpaces_ - 6, numPossiblePuyos));
        int newChains = min((numPossiblePuyos / 4), 19);
        // TODO(mayah): newChains should not be negative. restFrames is negative?
        if (newChains < 0)
            newChains = 0;

        maxScore = std::max(maxScore, ACCUMULATED_RENSA_SCORE[newChains]);
    }

    return maxScore;
}

string GazeResult::toRensaInfoString() const
{
    stringstream ss;
    ss << "next move frameId: " << frameIdToStartNextMove_ << endl;
    ss << "FeasibleRensaTree:" << endl;
    ss << feasibleRensaHandTree_.toString();
    ss << "PossibleRensaTree:" << endl;
    ss << possibleRensaHandTree_.toString();

    return ss.str();
}

// ----------------------------------------------------------------------

void Gazer::initialize(int frameIdGameWillBegin)
{
    gazeResult_.reset(frameIdGameWillBegin, 72);
}

void Gazer::gaze(int frameId, const CoreField& originalField, const KumipuyoSeq& kumipuyoSeq)
{
    LOG(INFO) << "Gaze: \n" << originalField.toDebugString() << "\nSeq: " << kumipuyoSeq.toString();

    int numReachableSpaces = originalField.countConnectedPuyos(3, 12);
    gazeResult_.reset(frameId, numReachableSpaces);

    // FeasibleRensaHandTree.
    {
        RensaHandNodeMaker maker(2, kumipuyoSeq);
        //vector<RensaHandEdge> edges;
        auto callback = [&](const CoreField& field, const std::vector<Decision>& decisions,
                            int /*numChigiri*/, int framesToIgnite, int lastDropFrames, bool shouldFire) {
            if (!shouldFire)
                return;

            CoreField cf(field);
            RensaCoefTracker tracker;
            RensaResult rensaResult = cf.simulate(&tracker);

            if (rensaResult.score < 70)
                return;

            IgnitionRensaResult ignitionRensaResult(rensaResult, framesToIgnite, lastDropFrames);

            PuyoSet usedPuyoSet;
            for (size_t i = 0; i < decisions.size(); ++i) {
                usedPuyoSet.add(kumipuyoSeq.axis(i));
                usedPuyoSet.add(kumipuyoSeq.child(i));
            }

            RensaHandCandidate candidate(ignitionRensaResult, tracker.result(), cf, usedPuyoSet, 0);
            maker.addCandidate(candidate);
        };

        int maxDepth = std::min<int>(3, kumipuyoSeq.size());
        Plan::iterateAvailablePlansWithoutFiring(originalField, kumipuyoSeq, maxDepth, callback);

        RensaHandTree tree = RensaHandTree(std::vector<RensaHandNode> { maker.makeNode() });
        LOG(INFO) << "Feasible: " << endl << tree.toString();
        gazeResult_.setFeasibleRensaHandTree(std::move(tree));
    }

    // PossibleRensaHandTree.
    // We'd like make the depth 3, but eval() gets really slow (2~3 ms each hand.)
    RensaHandTree tree = RensaHandTree::makeTree(2, originalField, PuyoSet(), 0, kumipuyoSeq);
    LOG(INFO) << "Possible:" << endl << tree.toString();

    gazeResult_.setPossibleRensaHandTree(std::move(tree));
}
