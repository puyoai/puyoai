#include "yukina_ai.h"

#include <iostream>

DECLARE_bool(from_wrapper);

using namespace std;

YukinaAI::YukinaAI(int argc, char* argv[]) :
    MayahBaseAI(argc, argv, "yukina", Executor::makeDefaultExecutor())
{
    if (!FLAGS_from_wrapper) {
        LOG(ERROR) << "mayah was not run with run.sh?" << endl
                   << "Use run.sh instead of using mayah_cpu directly.";
    }

    setBehaviorRethinkAfterOpponentRensa(true);
}

DropDecision YukinaAI::think(int frame_id, const CoreField& field, const KumipuyoSeq& kumipuyo_seq,
                             const PlayerState& me, const PlayerState& enemy, bool fast) const
{
    const bool usesDecisionBook = true;
    const bool usesRensaHandTree = false;

    if (enemy.isRensaOngoing()) {
        return pattern_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, gazer_.gazeResult(), fast,
                                       usesDecisionBook, usesRensaHandTree);
    }

    // Turning the table mode
    if (field.countColor(PuyoColor::OJAMA) >= 16) {
        return rush_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, fast);
    }
    if (enemy.field.countColor(PuyoColor::OJAMA) >= 30) {
        return rush_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, fast);
    }
    {
        int my_field_puyo = field.countPuyos();
        int enemy_field_puyo = enemy.field.countPuyos();
        int diff = my_field_puyo - enemy_field_puyo;
        if (diff >= 24 || diff <= -24) {
            return rush_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, fast);
        }
    }

    if (field.countPuyos() <= 24 || field.countPuyos() >= 64) {
        return pattern_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, gazer_.gazeResult(), fast,
                                       usesDecisionBook, usesRensaHandTree);
    }

    return beam_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, fast);
}
