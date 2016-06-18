#include "yukina_ai.h"

#include <iostream>

using namespace std;

YukinaAI::YukinaAI(int argc, char* argv[]) :
    MayahBaseAI(argc, argv, "yukina", Executor::makeDefaultExecutor())
{
}

DropDecision YukinaAI::think(int frameId, const CoreField& field, const KumipuyoSeq& seq,
                             const PlayerState& me, const PlayerState& enemy, bool fast) const
{
    return thinkByBeamSearch(frameId, field, seq, me, enemy, fast);
}
