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
    double beginTimeSec = currentTime();
    DropDecision dd = thinkByThinker(frame_id, field, kumipuyo_seq, me, enemy, fast);
    if (dd.valid()) {
        double endTimeSec = currentTime();
        double durationSec = endTimeSec - beginTimeSec;

        const GazeResult& gazeResult = gazer_.gazeResult();

        std::stringstream ss;
        if (enemy.isRensaOngoing()) {
            ss << "Gazed (ongoing) : " << enemy.currentRensaResult.score
               << " in " << (enemy.rensaFinishingFrameId() - frame_id) << " / ";
        } else {
            ss << "Gazed = "
               << gazeResult.estimateMaxFeasibleScore(frame_id + 100, enemy)
               << " in " << 100 << " / "
               << gazeResult.estimateMaxFeasibleScore(frame_id + 300, enemy)
               << " in " << 300 << " / "
               << gazeResult.estimateMaxFeasibleScore(frame_id + 500, enemy)
               << " in " << 500 << " / ";
        }

        ss << "OJAMA: "
           << "fixed=" << me.fixedOjama << " "
           << "pending=" << me.pendingOjama << " "
           << "total=" << me.totalOjama(enemy) << " "
           << "frameId=" << me.rensaFinishingFrameId() << " / ";

        ss << (durationSec * 1000) << " [ms]";
        dd.setMessage(dd.message() + "\n" + ss.str());
        return dd;
    }

    // dd is invalid.
    // Rethink by pattern_thinker_ with fast=true.
    const bool usesDecisionBook = true;
    const bool usesRensaHandTree = false;
    return pattern_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, gazer_.gazeResult(), true,
                                   usesDecisionBook, usesRensaHandTree);
}

DropDecision YukinaAI::thinkByThinker(int frame_id, const CoreField& field, const KumipuyoSeq& kumipuyo_seq,
                                      const PlayerState& me, const PlayerState& enemy, bool fast) const
{
    const bool usesDecisionBook = true;
    const bool usesRensaHandTree = !fast;

    if (enemy.isRensaOngoing()) {
        return pattern_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, gazer_.gazeResult(), fast,
                                       usesDecisionBook, usesRensaHandTree);
    }

    if (field.countPuyos() >= 64) {
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

    if (field.countPuyos() <= 24) {
        return pattern_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, gazer_.gazeResult(), fast,
                                       usesDecisionBook, usesRensaHandTree);
    }

    return beam_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, fast);
}
