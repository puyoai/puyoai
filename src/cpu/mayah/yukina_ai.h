#ifndef CPU_MAYAH_YUKINA_AI_H_
#define CPU_MAYAH_YUKINA_AI_H_

#include <memory>
#include <set>
#include <vector>

#include "mayah_base_ai.h"

class YukinaAI : public MayahBaseAI {
public:
    YukinaAI(int argc, char* argv[]);

    DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const override;

    DropDecision thinkByThinker(int frameId, const CoreField&, const KumipuyoSeq&,
                                const PlayerState& me, const PlayerState& enemy, bool fast) const;

private:
    std::string gazeMessage(int frame_id, const PlayerState& me, const PlayerState& enemy, const GazeResult& gazeResult) const;

    mutable std::mutex mu_; // for cout
};

#endif // CPU_MAYAH_YUKINA_AI_H_
