#ifndef CPU_MAYAH_HAND_TREE_H_
#define CPU_MAYAH_HAND_TREE_H_

#include <ostream>
#include <string>
#include <vector>

#include "core/kumipuyo_seq.h"
#include "core/rensa_result.h"
#include "core/rensa_track_result.h"

class ColumnPuyoList;
class CoreField;
class KumipuyoSeq;
class PuyoSet;
struct RensaHandCandidate;

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
    RensaHandTree(const RensaHand& rensaHand, std::vector<RensaHandTree> tree) :
        rensaHand_(rensaHand), children_(std::move(tree)) {}

    static std::vector<RensaHandTree> makeTree(int restIteration,
                                               const CoreField& currentField,
                                               const PuyoSet& usedPuyoSet,
                                               int usedPuyoMoveFrames,
                                               const KumipuyoSeq& wholeKumipuyoSeq);

    static int eval(const std::vector<RensaHandTree>& myTree,
                    int myStartingFrameId,
                    int myNumOjama,
                    int myOjamaCommittingFrameId,
                    const std::vector<RensaHandTree>& enemyTree,
                    int enemyStartingFrameId,
                    int enemyNumOjama,
                    int enemyOjamaCommittingFrameId);

    const RensaHand& rensaHand() const { return rensaHand_; }
    const std::vector<RensaHandTree>& children() const { return children_; }

    std::string toString() const;
    void dump(int depth) const;
    void dumpTo(int depth, std::ostream* os) const;

private:
    RensaHand rensaHand_;
    std::vector<RensaHandTree> children_;
};

class RensaHandTreeMaker {
public:
    RensaHandTreeMaker(int restIteration, const KumipuyoSeq& kumipuyoSeq);
    ~RensaHandTreeMaker();

    int restIteration() const { return restIteration_; }

    RensaResult add(CoreField&& cf,
                    const ColumnPuyoList& puyosToComplement,
                    int usedPuyoMoveFrames,
                    const PuyoSet& usedPuyoSet);

    std::vector<RensaHandTree> makeSummary();

private:
    const int restIteration_;
    const KumipuyoSeq kumipuyoSeq_;
    std::vector<RensaHandCandidate> data_;
};

#endif // CPU_MAYAH_HAND_TREE_H_
