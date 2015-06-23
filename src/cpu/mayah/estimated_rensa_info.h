#ifndef CPU_MAYAH_ESTIMATED_RENSA_INFO_H_
#define CPU_MAYAH_ESTIMATED_RENSA_INFO_H_

#include <ostream>
#include <string>
#include <vector>

#include "core/rensa_result.h"
#include "core/rensa_track_result.h"

class CoreField;
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

#endif // CPU_MAYAH_ESTIMATED_RENSA_INFO_H_
