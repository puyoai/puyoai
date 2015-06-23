#ifndef CPU_MAYAH_HAND_TREE_H_
#define CPU_MAYAH_HAND_TREE_H_

#include <vector>

#include "core/kumipuyo_seq.h"

#include "estimated_rensa_info.h"

class ColumnPuyoList;
struct DetailEstimatedRensaInfo;
class PuyoSet;

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
