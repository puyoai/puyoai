#ifndef CPU_MAYAH_GAZER_H_
#define CPU_MAYAH_GAZER_H_

#include <map>
#include <string>
#include <vector>

#include "base/noncopyable.h"
#include "core/algorithm/puyo_set.h"
#include "core/client/ai/ai.h"

#include "rensa_hand_tree.h"

class KumipuyoSeq;

class GazeResult {
public:
    GazeResult() {}
    GazeResult(int frameIdGazedAt,
               int numReachableSpaces,
               const std::vector<RensaHand>& feasible,
               const std::vector<RensaHand>& possible) :
        frameIdGazedAt_(frameIdGazedAt),
        numReachableSpaces_(numReachableSpaces),
        feasibleRensaInfos_(feasible),
        possibleRensaInfos_(possible)
    {
    }

    int frameIdGazedAt() const { return frameIdGazedAt_; }

    // Returns the (expecting) possible max score by this frame.
    int estimateMaxScore(int frameId, const PlayerState& enemy) const;
    const std::vector<RensaHand>& feasibleRensaInfos() const { return feasibleRensaInfos_; }

    const std::vector<RensaHandTree>& rensaTree() const { return possibleRensaTree_; }

    void reset(int frameId, int numReachableSpaces);
    void setFeasibleRensaInfo(std::vector<RensaHand> infos) { feasibleRensaInfos_ = std::move(infos); }
    void setPossibleRensaInfo(std::vector<RensaHand> infos) { possibleRensaInfos_ = std::move(infos); }
    void setPossibleRensaInfoTree(std::vector<RensaHandTree> tree) { possibleRensaTree_ = std::move(tree); }

    std::string toRensaInfoString() const;

private:
    int estimateMaxScoreFromFeasibleRensas(int frameId) const;
    int estimateMaxScoreFromPossibleRensas(int frameId) const;
    int estimateMaxScoreFrom(int frameId, const std::vector<RensaHand>& rensaInfos) const;

    int frameIdGazedAt_ = -1;
    int numReachableSpaces_ = 72;
    // FiesibleRensa is the rensa that the enemy can really fire in current/next/nextnext tsumo.
    std::vector<RensaHand> feasibleRensaInfos_;
    // PossibleRensa is the rensa the enemy will build in future.
    std::vector<RensaHand> possibleRensaInfos_;
    std::vector<RensaHandTree> possibleRensaTree_;
};

class Gazer : noncopyable {
public:
    void initialize(int frameIdGameWillBegin);

    void gaze(int frameId, const CoreField&, const KumipuyoSeq&);

    const GazeResult& gazeResult() const { return gazeResult_; }

private:
    void updateFeasibleRensas(const CoreField&, const KumipuyoSeq&);
    void updatePossibleRensas(const CoreField&, const KumipuyoSeq&);

    GazeResult gazeResult_;
};

#endif // CPU_MAYAH_GAZER_H_
