#include "rensa_hand_tree.h"

#include <iostream>

#include "core/algorithm/puyo_set.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/algorithm/rensa_detector.h"
#include "core/core_field.h"
#include "core/frame.h"

using namespace std;

struct RensaHandCandidate {
    RensaHandCandidate() {}
    RensaHandCandidate(const IgnitionRensaResult& ignitionRensaResult,
                       const CoreField& fieldAfterRensa,
                       const PuyoSet& puyoSet,
                       int wholeFramesToIgnite,
                       const RensaCoefResult& coefResult) :
        ignitionRensaResult(ignitionRensaResult),
        fieldAfterRensa(fieldAfterRensa),
        puyoSet(puyoSet),
        wholeFramesToIgnite(wholeFramesToIgnite),
        coefResult(coefResult)
    {
    }

    int chains() const { return ignitionRensaResult.chains(); }
    int score() const { return ignitionRensaResult.score(); }
    int rensaFrames() const { return ignitionRensaResult.rensaFrames(); }
    int framesToIgnite() const { return ignitionRensaResult.framesToIgnite(); }

    int totalFrames() const { return ignitionRensaResult.totalFrames(); }

    std::string toString() const;

    IgnitionRensaResult ignitionRensaResult;
    CoreField fieldAfterRensa;
    PuyoSet puyoSet;
    int wholeFramesToIgnite;
    RensaCoefResult coefResult;
};

struct SortByTotalFrames {
    bool operator()(const RensaHandCandidate& lhs, const RensaHandCandidate& rhs) const
    {
        if (lhs.totalFrames() != rhs.totalFrames())
            return lhs.totalFrames() < rhs.totalFrames();

        // The rest of '>' is intentional.
        if (lhs.score() != rhs.score())
            return lhs.score() > rhs.score();
        if (lhs.chains() != rhs.chains())
            return lhs.score() > rhs.chains();

        if (lhs.framesToIgnite() != rhs.framesToIgnite())
            return lhs.framesToIgnite() < rhs.framesToIgnite();

        return lhs.coefResult.coef(lhs.chains()) > rhs.coefResult.coef(rhs.chains());
    }
};

string RensaHand::toString() const
{
    char buf[80];
    sprintf(buf, "totalFrames, chains, score, framesToIgnite = %d, %d, %d, %d", totalFrames(), chains(), score(), framesToIgnite());
    return buf;
}

string RensaHandTree::toString() const
{
    ostringstream oss;
    dumpTo(0, &oss);

    return oss.str();
}

void RensaHandTree::dump(int depth) const
{
    dumpTo(depth, &cout);
}

void RensaHandTree::dumpTo(int depth, ostream* os) const
{
    for (int i = 0; i < depth * 2; ++i)
        *os << ' ';
    *os << rensaHand.toString() << endl;

    for (const auto& t : children)
        t.dumpTo(depth + 1, os);
}

RensaHandTreeMaker::RensaHandTreeMaker(int restIteration, const KumipuyoSeq& kumipuyoSeq) :
    restIteration_(restIteration),
    kumipuyoSeq_(kumipuyoSeq)
{
}

RensaHandTreeMaker::~RensaHandTreeMaker()
{
}

// TODO(mayah): buggy.
// static
int RensaHandTree::eval(const vector<RensaHandTree>& myTree,
                        int myStartingFrameId,
                        int myNumOjama,
                        int myOjamaCommittingFrameId,
                        const vector<RensaHandTree>& enemyTree,
                        int enemyStartingFrameId,
                        int enemyNumOjama,
                        int enemyOjamaCommittingFrameId)
{
    // TODO(mayah): This definition is also there in HandTree.
    // Should we have some function to make this?
    const int NUM_FRAMES_OF_ONE_HAND = FRAMES_TO_DROP_FAST[8] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT;
    const int NUM_FRAMES_OF_ONE_RENSA = FRAMES_PREPARING_NEXT + FRAMES_VANISH_ANIMATION + FRAMES_GROUNDING + FRAMES_TO_DROP_FAST[1];

    if (myNumOjama > 0 && enemyNumOjama > 0) {
        if (myNumOjama >= enemyNumOjama) {
            myNumOjama -= enemyNumOjama;
            enemyNumOjama = 0;
        } else {
            enemyNumOjama -= myNumOjama;
            myNumOjama = 0;
        }
    }

    if (myNumOjama > 2) {
        if (myOjamaCommittingFrameId < myStartingFrameId) {
            return -myNumOjama;
        }

        int restFrames = myOjamaCommittingFrameId - myStartingFrameId;
        int numHands = restFrames / NUM_FRAMES_OF_ONE_HAND;
        int plusRensa = (numHands / 2) - 1;
        if (plusRensa < 0)
            plusRensa = 0;
        else if (19 < plusRensa)
            plusRensa = 19;

        int best = -myNumOjama;
        for (size_t i = 0; i < myTree.size(); ++i) {
            const RensaHandTree& chosen = myTree[i];
            if (myOjamaCommittingFrameId < myStartingFrameId + chosen.rensaHand.framesToIgnite()) {
                // In this case, we cannot fire this rensa.
                continue;
            }

            // Fire this immediately?
            if (myNumOjama < chosen.rensaHand.score() / 70) {
                int s = eval(chosen.children, myStartingFrameId + chosen.rensaHand.totalFrames(), 0, 0,
                                      enemyTree, enemyStartingFrameId, enemyNumOjama + chosen.rensaHand.score() / 70, myStartingFrameId + chosen.rensaHand.totalFrames());
                if (best < s)
                    best = s;
            } else {
                // in short...
                // TODO(mayah): Should we think enemy's OIUCHI when enemyTree is not empty?
                int s = chosen.rensaHand.score() / 70 - myNumOjama;
                if (best < s)
                    best = s;
            }

            // Fire this by making this large?
            if (plusRensa > 0) {
                int score = chosen.rensaHand.coefResult.score(plusRensa);
                if (myNumOjama < score / 70) {
                    int s = eval(chosen.children, myStartingFrameId + restFrames + chosen.rensaHand.totalFrames(), 0, 0,
                                          enemyTree, enemyStartingFrameId, enemyNumOjama + score / 70, myStartingFrameId + restFrames + chosen.rensaHand.totalFrames());
                    if (best < s)
                        best = s;
                }
            }
        }

        // Without using the current rensa?
        {
            int plusOjama = ACCUMULATED_RENSA_SCORE[plusRensa] / 70;
            if (myNumOjama > plusOjama) {
                int s = plusOjama - myNumOjama;
                if (best < s)
                    best = s;
            } else {
                int rensaFrame = NUM_FRAMES_OF_ONE_RENSA * plusRensa;
                int s = eval(myTree, myStartingFrameId + rensaFrame, 0, 0,
                             enemyTree, enemyStartingFrameId, enemyNumOjama + plusOjama - myNumOjama, myStartingFrameId + rensaFrame);
                if (best < s)
                    best = s;
            }
        }

        return best;
    } else if (enemyNumOjama > 2) {
        if (enemyOjamaCommittingFrameId < enemyStartingFrameId) {
            return enemyNumOjama;
        }

        return -eval(enemyTree, enemyStartingFrameId, enemyNumOjama, enemyOjamaCommittingFrameId,
                     myTree, myStartingFrameId, myNumOjama, myOjamaCommittingFrameId);
    } else {
        int best = -10000;
        int worst = 0;

        // choose the best hand from my hand.
        for (size_t i = 0; i < myTree.size(); ++i) {
            const RensaHandTree& chosen = myTree[i];
            int s = eval(chosen.children, myStartingFrameId + chosen.rensaHand.totalFrames(), 0, 0,
                         enemyTree, enemyStartingFrameId, chosen.rensaHand.score() / 70, myStartingFrameId + chosen.rensaHand.totalFrames());
            if (best < s)
                best = s;
        }

        // enemy will choose the worst one for me.
        for (size_t i = 0; i < enemyTree.size(); ++i) {
            const RensaHandTree& chosen = enemyTree[i];
            int s = eval(myTree, myStartingFrameId, chosen.rensaHand.score() / 70, enemyStartingFrameId + chosen.rensaHand.totalFrames(),
                         chosen.children, enemyStartingFrameId + chosen.rensaHand.totalFrames(), 0, 0);
            if (s < worst)
                worst = s;
        }

        // my best
        return std::max(best, worst);
    }

    return 0;
}

RensaResult RensaHandTreeMaker::add(CoreField&& cf,
                                    const ColumnPuyoList& puyosToComplement,
                                    int usedPuyoMoveFrames,
                                    const PuyoSet& usedPuyoSet)
{
    const int NUM_FRAMES_OF_ONE_HAND = FRAMES_TO_DROP_FAST[8] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT;

    RensaCoefTracker tracker;
    RensaResult rensaResult = cf.simulate(&tracker);

    // skip.
    if (rensaResult.score < 70)
        return rensaResult;

    PuyoSet wholePuyoSet(usedPuyoSet);
    wholePuyoSet.add(puyosToComplement);

    int necessaryPuyos = PuyoPossibility::necessaryPuyos(wholePuyoSet, kumipuyoSeq_, 0.5);
    int necessaryHands = (necessaryPuyos + 1) / 2;
    // We need to remove last hand frames, since we'd like to calculate framesToIgnite.
    if (necessaryHands > 1)
        necessaryHands -= 1;

    // Estimate the number of frames to initiate this rensa.
    // 8 is arbitrarily chosen.
    int wholeFramesToIgnite = NUM_FRAMES_OF_ONE_HAND * necessaryHands;
    int framesToIgnite = wholeFramesToIgnite - usedPuyoMoveFrames;
    if (framesToIgnite < NUM_FRAMES_OF_ONE_HAND)
        framesToIgnite = NUM_FRAMES_OF_ONE_HAND;
    data_.emplace_back(IgnitionRensaResult(rensaResult, framesToIgnite), cf, wholePuyoSet, wholeFramesToIgnite, tracker.result());

    return rensaResult;
}

vector<RensaHandTree> RensaHandTreeMaker::makeSummary()
{
    if (data_.empty())
        return vector<RensaHandTree>();

    sort(data_.begin(), data_.end(), SortByTotalFrames());

    vector<RensaHandTree> tree;
    {
        {
            const RensaHandCandidate& info = data_.front();
            RensaHandTree t {
                RensaHand(info.ignitionRensaResult, info.coefResult),
                RensaHandTree::makeTree(restIteration() - 1, info.fieldAfterRensa, info.puyoSet, info.wholeFramesToIgnite, kumipuyoSeq_),
            };
            tree.push_back(t);
        }

        for (const RensaHandCandidate& info : data_) {
            // Don't consider if chain side is too close.
            if (info.score() <= tree.back().rensaHand.score() + 140)
                continue;

            DCHECK(tree.back().rensaHand.totalFrames() < info.totalFrames());
            RensaHandTree t {
                RensaHand(info.ignitionRensaResult, info.coefResult),
                RensaHandTree::makeTree(restIteration() - 1, info.fieldAfterRensa, info.puyoSet, info.wholeFramesToIgnite, kumipuyoSeq_),
            };
            tree.push_back(t);
        }
    }

    return std::move(tree);
}

//
vector<RensaHandTree> RensaHandTree::makeTree(int restIteration,
                                                       const CoreField& currentField,
                                                       const PuyoSet& usedPuyoSet,
                                                       int usedPuyoMoveFrames,
                                                       const KumipuyoSeq& wholeKumipuyoSeq)
{
    if (restIteration <= 0)
        return vector<RensaHandTree>();

    RensaHandTreeMaker maker(restIteration, wholeKumipuyoSeq);
    auto callback = [&](CoreField&& cf, const ColumnPuyoList& puyosToComplement) -> RensaResult {
        return maker.add(std::move(cf), puyosToComplement, usedPuyoMoveFrames, usedPuyoSet);
    };
    RensaDetector::detectIteratively(currentField, RensaDetectorStrategy::defaultFloatStrategy(), 3, callback);
    return maker.makeSummary();
}
