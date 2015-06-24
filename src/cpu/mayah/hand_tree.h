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
struct DetailEstimatedRensaInfo;
class KumipuyoSeq;
class PuyoSet;

struct EstimatedRensaInfo {
    EstimatedRensaInfo() {}
    EstimatedRensaInfo(const IgnitionRensaResult& ignitionRensaResult,
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

struct EstimatedRensaInfoTree {
    EstimatedRensaInfoTree() {}
    EstimatedRensaInfoTree(const EstimatedRensaInfo& info, std::vector<EstimatedRensaInfoTree> tree) :
        estimatedRensaInfo(info), children(std::move(tree)) {}

    std::string toString() const;
    void dump(int depth) const;
    void dumpTo(int depth, std::ostream* os) const;

    EstimatedRensaInfo estimatedRensaInfo;
    std::vector<EstimatedRensaInfoTree> children;
};

class HandTree {
public:
    static std::vector<EstimatedRensaInfoTree> makeTree(int restIteration,
                                                        const CoreField& currentField,
                                                        const PuyoSet& usedPuyoSet,
                                                        int usedPuyoMoveFrames,
                                                        const KumipuyoSeq& wholeKumipuyoSeq);

    static int eval(const std::vector<EstimatedRensaInfoTree>& myTree,
                    int myStartingFrameId,
                    int myNumOjama,
                    int myOjamaCommittingFrameId,
                    const std::vector<EstimatedRensaInfoTree>& enemyTree,
                    int enemyStartingFrameId,
                    int enemyNumOjama,
                    int enemyOjamaCommittingFrameId);
};

class HandTreeMaker {
public:
    HandTreeMaker(int restIteration, const KumipuyoSeq& kumipuyoSeq);
    ~HandTreeMaker();

    int restIteration() const { return restIteration_; }

    RensaResult add(CoreField&& cf,
                    const ColumnPuyoList& puyosToComplement,
                    int usedPuyoMoveFrames,
                    const PuyoSet& usedPuyoSet);

    std::vector<EstimatedRensaInfoTree> makeSummary();

private:
    const int restIteration_;
    const KumipuyoSeq kumipuyoSeq_;
    std::vector<DetailEstimatedRensaInfo> data_;
};

#endif // CPU_MAYAH_HAND_TREE_H_
