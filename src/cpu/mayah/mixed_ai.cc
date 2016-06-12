#include "mixed_ai.h"

MixedAI::MixedAI(int argc, char* argv[]) :
    AI(argc, argv, "mixed"),
    ai_(argc, argv),
    yukinaAi_(argc, argv)
{
    ai_.setUsesRensaHandTree(false);
    ai_.removeNontokopuyoParameter();
}

DropDecision MixedAI::think(int frameId, const CoreField& field, const KumipuyoSeq& seq,
                            const PlayerState& me, const PlayerState& enemy, bool fast) const
{
    if (field.countPuyos() <= 24 || field.countPuyos() >= 64)
        return ai_.think(frameId, field, seq, me, enemy, fast);
     else
        return yukinaAi_.think(frameId, field, seq, me, enemy, fast);
}
