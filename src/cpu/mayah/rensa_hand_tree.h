#ifndef CPU_MAYAH_HAND_TREE_H_
#define CPU_MAYAH_HAND_TREE_H_

#include <ostream>
#include <string>
#include <vector>

#include "core/frame.h"
#include "core/kumipuyo_seq.h"
#include "core/rensa_result.h"
#include "core/rensa_track_result.h"

class ColumnPuyoList;
class CoreField;
class KumipuyoSeq;
class PuyoSet;

struct RensaHandCandidate;
class RensaHandEdge;
class RensaHandNode;
class RensaHandTree;

// These values are arbitrary chosen.
const int NUM_FRAMES_OF_ONE_HAND = FRAMES_TO_DROP_FAST[8] + FRAMES_GROUNDING + FRAMES_PREPARING_NEXT;
const int NUM_FRAMES_OF_ONE_RENSA = FRAMES_VANISH_ANIMATION + FRAMES_GROUNDING + FRAMES_TO_DROP_FAST[1];

struct RensaHand {
    RensaHand() {}
    RensaHand(const IgnitionRensaResult& ignitionRensaResult,
              const RensaCoefResult& coefResult) :
        ignitionRensaResult(ignitionRensaResult),
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
    RensaCoefResult coefResult;
};

class RensaHandTree {
public:
    RensaHandTree() {}
    explicit RensaHandTree(std::vector<RensaHandNode> nodes) :
        nodes_(nodes) {}

    static RensaHandTree makeTree(int restIteration,
                                  const CoreField& currentField,
                                  const PuyoSet& usedPuyoSet,
                                  int usedPuyoMoveFrames,
                                  const KumipuyoSeq& wholeKumipuyoSeq);

    static int eval(const RensaHandTree& myTree,
                    int myStartingFrameId,
                    int myOjamaIndex,
                    int myNumOjama,
                    int myOjamaCommittingFrameId,
                    const RensaHandTree& enemyTree,
                    int enemyStartingFrameId,
                    int enemyOjamaIndex,
                    int enemyNumOjama,
                    int enemyOjamaCommittingFrameId);

    const std::vector<RensaHandNode>& nodes() const { return nodes_; }
    const RensaHandNode& node(int index) const { return nodes_[index]; }

    std::string toString() const;
    void dump(int depth) const;
    void dumpTo(int depth, std::ostream* os) const;

private:
    std::vector<RensaHandNode> nodes_;  // by ojama lines
};

class RensaHandEdge {
public:
    RensaHandEdge(const RensaHand& rensaHand, const RensaHandTree& tree) :
        rensaHand_(rensaHand),
        tree_(tree)
    {
    }

    const RensaHand& rensaHand() const { return rensaHand_; }
    const RensaHandTree& tree() const { return tree_; }

private:
    RensaHand rensaHand_;
    RensaHandTree tree_;
};

class RensaHandNode {
public:
    RensaHandNode() {}
    explicit RensaHandNode(std::vector<RensaHandEdge> edges) :
        edges_(std::move(edges)) {}

    const std::vector<RensaHandEdge>& edges() const { return edges_; }

private:
    std::vector<RensaHandEdge> edges_;
};

class RensaHandNodeMaker {
public:
    RensaHandNodeMaker(int restIteration, const KumipuyoSeq& kumipuyoSeq);
    ~RensaHandNodeMaker();

    int restIteration() const { return restIteration_; }

    RensaResult add(CoreField&& cf,
                    const ColumnPuyoList& puyosToComplement,
                    int usedPuyoMoveFrames,
                    const PuyoSet& usedPuyoSet);

    RensaHandNode makeNode();

private:
    const int restIteration_;
    const KumipuyoSeq kumipuyoSeq_;
    std::vector<RensaHandCandidate> data_;
};

#endif // CPU_MAYAH_HAND_TREE_H_
