#ifndef CPU_MAYAH_GAZER_H_
#define CPU_MAYAH_GAZER_H_

#include <map>
#include <string>
#include <vector>

#include "base/noncopyable.h"
#include "core/client/ai/ai.h"
#include "core/probability/puyo_set.h"

#include "rensa_hand_tree.h"

class KumipuyoSeq;

class GazeResult {
public:
    GazeResult() {}

    int frameIdToStartNextMove() const { return frameIdToStartNextMove_; }

    const RensaHandTree& feasibleRensaHandTree() const { return feasibleRensaHandTree_; }
    const RensaHandTree& possibleRensaHandTree() const { return possibleRensaHandTree_; }

    // Returns the (expecting) possible max score by this frame.
    int estimateMaxScore(int frameId, const PlayerState& enemy) const;

    void setFeasibleRensaHandTree(RensaHandTree tree) { feasibleRensaHandTree_ = std::move(tree); }
    void setPossibleRensaHandTree(RensaHandTree tree) { possibleRensaHandTree_ = std::move(tree); }

    void reset(int frameIdToStartNextMove, int numReachableSpaces);

    std::string toRensaInfoString() const;

private:
    int estimateMaxScoreFromFeasibleRensas(int frameId) const;
    int estimateMaxScoreFromPossibleRensas(int frameId) const;

    int frameIdToStartNextMove_ = -1;
    int numReachableSpaces_ = 72;

    RensaHandTree feasibleRensaHandTree_;
    RensaHandTree possibleRensaHandTree_;
};

class Gazer : noncopyable {
public:
    void initialize(int frameIdGameWillBegin);
    void gaze(int frameId, const CoreField&, const KumipuyoSeq&);

    const GazeResult& gazeResult() const { return gazeResult_; }
private:
    GazeResult gazeResult_;
};

#endif // CPU_MAYAH_GAZER_H_
