#include "rensa_hand_tree.h"

#include <iostream>
#include <sstream>

#include "core/rensa/rensa_detector.h"
#include "core/core_field.h"
#include "core/frame.h"
#include "core/probability/column_puyo_list_probability.h"
#include "core/probability/puyo_set_probability.h"
#include "core/probability/puyo_set.h"

using namespace std;

namespace {

// TODO(mayah): more accurate number?
static const int FRAMES_TO_DIG[6] = {
     0,
     5 * NUM_FRAMES_OF_ONE_HAND,
     7 * NUM_FRAMES_OF_ONE_HAND + 1 * NUM_FRAMES_OF_ONE_RENSA,
     9 * NUM_FRAMES_OF_ONE_HAND + 2 * NUM_FRAMES_OF_ONE_RENSA,
    11 * NUM_FRAMES_OF_ONE_HAND + 3 * NUM_FRAMES_OF_ONE_RENSA,
    13 * NUM_FRAMES_OF_ONE_HAND + 4 * NUM_FRAMES_OF_ONE_RENSA,
};

} // namespace

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
    if (nodes().empty())
        return;

    for (const auto& edge : node(0).edges()) {
        for (int i = 0; i < depth * 2; ++i)
            *os << ' ';
        *os << edge.rensaHand().toString() << endl;
        edge.tree().dumpTo(depth + 1, os);
    }
}

// static
RensaHandTree RensaHandTree::makeTree(int restIteration,
                                      const CoreField& currentField,
                                      const PuyoSet& usedPuyoSet,
                                      int usedPuyoMoveFrames,
                                      const KumipuyoSeq& wholeKumipuyoSeq)
{
    if (restIteration <= 0)
        return RensaHandTree();

    vector<RensaHandNode> nodes(6);
    for (int ojamaLines = 0; ojamaLines <= 5; ++ojamaLines) {
        CoreField field(currentField);
        const int dropFrames = field.fallOjama(ojamaLines);

        RensaHandNodeMaker maker(restIteration, wholeKumipuyoSeq);
        auto callback = [&](CoreField&& cf, const ColumnPuyoList& puyosToComplement) -> RensaResult {
            return maker.add(std::move(cf), puyosToComplement, usedPuyoMoveFrames + dropFrames, usedPuyoSet);
        };
        RensaDetector::detectIteratively(field, RensaDetectorStrategy::defaultDropStrategy(), 3, callback);
        nodes[ojamaLines] = maker.makeNode();
    }

    return RensaHandTree(std::move(nodes));
}

// static
int RensaHandTree::eval(const RensaHandTree& myTree,
                        int myStartingFrameId,
                        int myOjamaLineIndex,
                        int myNumOjama,
                        int myOjamaCommittingFrameId,
                        const RensaHandTree& enemyTree,
                        int enemyStartingFrameId,
                        int enemyOjamaLineIndex,
                        int enemyNumOjama,
                        int enemyOjamaCommittingFrameId)
{
    DCHECK(0 <= myOjamaLineIndex && myOjamaLineIndex <= 5) << myOjamaLineIndex;
    DCHECK(0 <= enemyOjamaLineIndex && enemyOjamaLineIndex <= 5) << enemyOjamaLineIndex;

    if (myNumOjama > 0 && enemyNumOjama > 0) {
        if (myNumOjama >= enemyNumOjama) {
            myNumOjama -= enemyNumOjama;
            enemyNumOjama = 0;
        } else {
            enemyNumOjama -= myNumOjama;
            myNumOjama = 0;
        }
    }

    // I need to fire something.
    if (myNumOjama > 2) {
        int best = -1000;

        // Fire rensa before ojama if possible.
        for (int ojamaLines = 0; ojamaLines <= myOjamaLineIndex; ++ojamaLines) {
            int framesToDig = FRAMES_TO_DIG[myOjamaLineIndex - ojamaLines];
            if (myTree.nodes().size() <= static_cast<size_t>(ojamaLines)) {
                // No such nodes.
                continue;
            }

            for (const auto& edge : myTree.node(ojamaLines).edges()) {
                const RensaHand& rensaHand = edge.rensaHand();

                // Cannot fire this rensa?
                int restFrames = myOjamaCommittingFrameId - myStartingFrameId - framesToDig - rensaHand.framesToIgnite();
                if (restFrames <= 0)
                    continue;

                // Fire this immediately.
                if (myNumOjama < rensaHand.score() / 70) {
                    int plusOjama = rensaHand.score() / 70 - myNumOjama;
                    int finishingFrameId = myStartingFrameId + rensaHand.totalFrames();
                    int s = eval(edge.tree(), finishingFrameId, 0, 0, 0,
                                 enemyTree, enemyStartingFrameId, enemyOjamaLineIndex, enemyNumOjama + plusOjama, finishingFrameId);
                    if (best < s)
                        best = s;
                } else {
                    // Fired, but the amount is short.
                    int fallOjamaAmount = myNumOjama - rensaHand.score() / 70;
                    int fallOjamaLine = (fallOjamaAmount + 4) / 6;
                    if (fallOjamaLine <= 5) {
                        int fallOjamaFrames = FRAMES_TO_DROP[6] + framesGroundingOjama(fallOjamaAmount);
                        int finishingFrameId = myStartingFrameId + rensaHand.totalFrames() + fallOjamaFrames;
                        int s = eval(edge.tree(), finishingFrameId, fallOjamaLine, 0, 0,
                                     enemyTree, enemyStartingFrameId, enemyOjamaLineIndex, enemyNumOjama, enemyOjamaCommittingFrameId);
                        if (best < s)
                            best = s;
                    }
                }

                // Fire after making this large.
                int plusRensa = std::min(4, (restFrames / NUM_FRAMES_OF_ONE_HAND) / 2 - 1);
                if (plusRensa > 0) {
                    int score = rensaHand.coefResult.score(plusRensa);
                    if (myNumOjama < score / 70) {
                        int plusOjama = score / 70 - myNumOjama;
                        int finishingFrameId = myOjamaCommittingFrameId + rensaHand.totalFrames();
                        int s = eval(edge.tree(), finishingFrameId, 0, 0, 0,
                                     enemyTree, enemyStartingFrameId, enemyOjamaLineIndex, enemyNumOjama + plusOjama, finishingFrameId);
                        if (best < s)
                            best = s;
                    }
                }
            }
        }

        // Fire rensa without using the current rensa.
#if 0
        {
            int restFrames = myOjamaCommittingFrameId - myStartingFrameId;
            int numHands = restFrames / NUM_FRAMES_OF_ONE_HAND;
            int plusRensa = (numHands / 2) - 1;
            if (plusRensa < 0)
                plusRensa = 0;
            else if (19 < plusRensa)
                plusRensa = 19;

            int plusOjama = ACCUMULATED_RENSA_SCORE[plusRensa] / 70;
            if (plusOjama >= 5) {
                if (myNumOjama > plusOjama) {
                    int s = plusOjama - myNumOjama;
                    // TODO(mayah): Need to eval again after this?
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
        }
#endif

        // Ojama is comitted.
        // If we got more then 60 ojama, we cannot do anything.
        if (myNumOjama <= 30) {
            int newMyOjamaLineIndex = myOjamaLineIndex + (myNumOjama + 4) / 6;
            if (5 < newMyOjamaLineIndex) {
                // We cannot do anything.
                best = std::max(best, -newMyOjamaLineIndex * 6);
            } else if (myOjamaLineIndex < newMyOjamaLineIndex) {
                int fallOjamaFrames = FRAMES_TO_DROP[6] + framesGroundingOjama(myNumOjama);
                int s = eval(myTree, myStartingFrameId + fallOjamaFrames, newMyOjamaLineIndex, 0, 0,
                             enemyTree, enemyStartingFrameId, enemyOjamaLineIndex, 0, 0);
                // Since we got |myNumOjama|, we need to reduce the score.
                s -= myNumOjama;
                if (best < s)
                    best = s;
            }
        } else {
            best = std::max(best, -myNumOjama);
        }

        return best;
    } else if (enemyNumOjama > 2) {
        return -eval(enemyTree, enemyStartingFrameId, enemyOjamaLineIndex, enemyNumOjama, enemyOjamaCommittingFrameId,
                     myTree, myStartingFrameId, myOjamaLineIndex, myNumOjama, myOjamaCommittingFrameId);
    } else {
        int best = -10000;
        int worst = 0;

        struct Candidate {
            int frameIdToIgnite;
            int frameIdToFinish;
            bool me;
            const RensaHandEdge* edge;
        };

        struct SortByFrameIdToFinish {
            bool operator()(const Candidate& lhs, const Candidate& rhs) const {
                if (lhs.frameIdToFinish != rhs.frameIdToFinish)
                    return lhs.frameIdToFinish < rhs.frameIdToFinish;
                if (lhs.frameIdToIgnite != rhs.frameIdToIgnite)
                    return lhs.frameIdToIgnite < rhs.frameIdToIgnite;
                if (lhs.me != rhs.me)
                    return lhs.me < rhs.me;
                return false;
            }
        };

        vector<Candidate> candidates;

        for (int ojamaLines = 0; ojamaLines <= myOjamaLineIndex; ++ojamaLines) {
            int framesToDig = FRAMES_TO_DIG[myOjamaLineIndex - ojamaLines];
            if (myTree.nodes().size() <= static_cast<size_t>(ojamaLines)) {
                // No such nodes.
                continue;
            }

            for (const auto& edge : myTree.node(ojamaLines).edges()) {
                const RensaHand& rensaHand = edge.rensaHand();
                int frameIdToIgnite = myStartingFrameId + framesToDig + rensaHand.framesToIgnite();
                int finishingFrameId = myStartingFrameId + rensaHand.totalFrames() + framesToDig;
                candidates.push_back(Candidate {
                    frameIdToIgnite, finishingFrameId, true, &edge
                });
            }
        }

        // choose the best hand from my hand.
        for (int ojamaLines = 0; ojamaLines <= enemyOjamaLineIndex; ++ojamaLines) {
            int framesToDig = FRAMES_TO_DIG[enemyOjamaLineIndex - ojamaLines];
            if (enemyTree.nodes().size() <= static_cast<size_t>(ojamaLines)) {
                // No such nodes.
                continue;
            }

            for (const auto& edge : enemyTree.node(ojamaLines).edges()) {
                const RensaHand& rensaHand = edge.rensaHand();
                int frameIdToIgnite = enemyStartingFrameId + framesToDig + rensaHand.framesToIgnite();
                int finishingFrameId = enemyStartingFrameId + framesToDig + rensaHand.totalFrames();
                candidates.push_back(Candidate {
                    frameIdToIgnite, finishingFrameId, false, &edge
                });
            }
        }

        std::sort(candidates.begin(), candidates.end(), SortByFrameIdToFinish());
        int myFastFinishingFrameId = 1000000;
        int enemyFastFinishingFrameId = 1000000;

        for (const auto& candidate : candidates) {
            if (candidate.me) {
                if (enemyFastFinishingFrameId < candidate.frameIdToIgnite)
                    continue;

                const RensaHand& rensaHand = candidate.edge->rensaHand();
                int ojama = rensaHand.score() / 70;
                int s = eval(candidate.edge->tree(), candidate.frameIdToFinish, 0, 0, 0,
                             enemyTree, enemyStartingFrameId, enemyOjamaLineIndex, ojama, candidate.frameIdToFinish);
                if (best < s)
                    best = s;
                if (6 <= ojama && candidate.frameIdToFinish < myFastFinishingFrameId)
                    myFastFinishingFrameId = candidate.frameIdToFinish;
            } else {
                if (myFastFinishingFrameId < candidate.frameIdToIgnite)
                    continue;

                const RensaHand& rensaHand = candidate.edge->rensaHand();
                int ojama = rensaHand.score() / 70;
                int s = eval(myTree, myStartingFrameId, myOjamaLineIndex, ojama, candidate.frameIdToFinish,
                             candidate.edge->tree(), candidate.frameIdToFinish, 0, 0, 0);
                if (s < worst)
                    worst = s;
                if (6 <= ojama && candidate.frameIdToFinish < enemyFastFinishingFrameId)
                    enemyFastFinishingFrameId = candidate.frameIdToFinish;
            }
        }

        // Choose the best hand.
        return std::max(best, worst);
    }

    return 0;
}

RensaHandNodeMaker::RensaHandNodeMaker(int restIteration, const KumipuyoSeq& kumipuyoSeq) :
    restIteration_(restIteration),
    kumipuyoSeq_(kumipuyoSeq)
{
}

RensaHandNodeMaker::~RensaHandNodeMaker()
{
}

RensaResult RensaHandNodeMaker::add(CoreField&& cf,
                                    const ColumnPuyoList& puyosToComplement,
                                    int usedFramesToMovePuyo,
                                    const PuyoSet& usedPuyoSet)
{
    RensaCoefTracker tracker;
    RensaResult rensaResult = cf.simulate(&tracker);

    // skip.
    if (rensaResult.score < 70)
        return rensaResult;

    PuyoSet wholeUsedPuyoSet(usedPuyoSet);
    wholeUsedPuyoSet.add(puyosToComplement);

    int necessaryKumipuyos = static_cast<int>(std::ceil(ColumnPuyoListProbability::instanceSlow()->necessaryKumipuyos(puyosToComplement)));
    //int necessaryPuyos = PuyoSetProbability::necessaryPuyos(wholeUsedPuyoSet, kumipuyoSeq_, 0.5);
    //int necessaryHands = (necessaryPuyos + 1) / 2;

    // Estimate the number of frames to initiate this rensa.
    int wholeFramesToIgnite = NUM_FRAMES_OF_ONE_HAND * necessaryKumipuyos;
    int framesToIgnite = wholeFramesToIgnite - usedFramesToMovePuyo - NUM_FRAMES_OF_ONE_HAND;
    if (framesToIgnite < 0)
        framesToIgnite = 0;
    data_.emplace_back(IgnitionRensaResult(rensaResult, framesToIgnite, NUM_FRAMES_OF_ONE_HAND),
                       tracker.result(), cf, wholeUsedPuyoSet, wholeFramesToIgnite);

    return rensaResult;
}

RensaHandNode RensaHandNodeMaker::makeNode()
{
    if (data_.empty())
        return RensaHandNode();

    sort(data_.begin(), data_.end(), SortByTotalFrames());

    vector<RensaHandEdge> edges;
    for (const RensaHandCandidate& info : data_) {
        // Don't consider if chain side is too close.
        if (!edges.empty() && info.score() <= edges.back().rensaHand().score() + 140)
            continue;

        DCHECK(edges.empty() || edges.back().rensaHand().totalFrames() < info.totalFrames());
        edges.emplace_back(RensaHand(info.ignitionRensaResult, info.coefResult),
                           RensaHandTree::makeTree(restIteration() - 1,
                                                   info.fieldAfterRensa,
                                                   info.alreadyUsedPuyoSet,
                                                   info.alreadyConsumedFramesToMovePuyo,
                                                   kumipuyoSeq_));
    }
    return RensaHandNode(std::move(edges));
}
