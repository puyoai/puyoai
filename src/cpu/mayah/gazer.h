#ifndef CPU_MAYAH_GAZER_H_
#define CPU_MAYAH_GAZER_H_

#include <map>
#include <string>
#include <vector>

#include "base/noncopyable.h"
#include "core/client/ai/ai.h"
#include "core/rensa_result.h"

class KumipuyoSeq;

struct EstimatedRensaInfo {
    EstimatedRensaInfo() {}
    EstimatedRensaInfo(int chains, int score, int framesToIgnite, const RensaCoefResult& coefResult) :
        chains(chains), score(score), framesToIgnite(framesToIgnite), coefResult(coefResult)
    {
    }

    std::string toString() const;

    int chains;
    int score;
    int framesToIgnite;
    RensaCoefResult coefResult;
};

class GazeResult {
public:
    GazeResult(int frameIdGazedAt, int restEmptyField,
               const std::vector<EstimatedRensaInfo>& feasible,
               const std::vector<EstimatedRensaInfo>& possible) :
        frameIdGazedAt_(frameIdGazedAt),
        restEmptyField_(restEmptyField),
        feasibleRensaInfos_(feasible),
        possibleRensaInfos_(possible)
    {
    }

    int frameIdGazedAt() const { return frameIdGazedAt_; }

    // Returns the (expecting) possible max score by this frame.
    int estimateMaxScore(int frameId, const PlayerState& enemy) const;

    std::string toRensaInfoString() const;

private:
    int estimateMaxScoreFromFeasibleRensas(int frameId) const;
    int estimateMaxScoreFromPossibleRensas(int frameId) const;
    int estimateMaxScoreFrom(int frameId, const std::vector<EstimatedRensaInfo>& rensaInfos) const;

    int frameIdGazedAt_;
    int restEmptyField_;

    std::vector<EstimatedRensaInfo> feasibleRensaInfos_;
    std::vector<EstimatedRensaInfo> possibleRensaInfos_;
};

class Gazer : noncopyable {
public:
    void initialize(int frameIdGameWillBegin);

    void setFrameIdGazedAt(int frameId) { frameIdGazedAt_ = frameId; }
    int frameIdGazedAt() const { return frameIdGazedAt_; }

    void gaze(int frameId, const CoreField&, const KumipuyoSeq&);

    GazeResult gazeResult() const;

private:
    void updateFeasibleRensas(const CoreField&, const KumipuyoSeq&);
    void updatePossibleRensas(const CoreField&, const KumipuyoSeq&);

    const std::vector<EstimatedRensaInfo>& possibleRensaInfos() const { return possibleRensaInfos_; }
    const std::vector<EstimatedRensaInfo>& feasibleRensaInfos() const { return feasibleRensaInfos_; }

    int frameIdGazedAt_ = -1;
    int restEmptyField_ = -1;

    // --- For these rensaInfos, frames means the framesToIgnite.
    // Fiesible Rensa is the Rensa the enemy can really fire in current/next/nextnext tsumo.
    std::vector<EstimatedRensaInfo> feasibleRensaInfos_;
    // Possible Rensa is the Rensa the enemy will build in future.
    std::vector<EstimatedRensaInfo> possibleRensaInfos_;
};

#endif // CPU_MAYAH_GAZER_H_
