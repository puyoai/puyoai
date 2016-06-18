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

private:
    mutable std::mutex mu_; // for cout
};

#endif // CPU_MAYAH_YUKINA_AI_H_
