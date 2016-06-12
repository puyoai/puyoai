#ifndef CPU_MAYAH_MIXED_AI_H_
#define CPU_MAYAH_MIXED_AI_H_

#include "core/client/ai/ai.h"

#include "mayah_ai.h"
#include "yukina_ai.h"

class MixedAI : public AI {
public:
    MixedAI(int argc, char* argv[]);

    DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const override;

private:
    DebuggableMayahAI ai_;
    YukinaAI yukinaAi_;
};

#endif // CPU_MAYAH_MIXED_AI_H_
