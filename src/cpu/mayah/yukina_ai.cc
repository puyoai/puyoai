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
    const GazeResult& gazeResult = gazer_.gazeResult();

    // tsubushi
    if (!enemy.isRensaOngoing()) {
        int len = std::min(kumipuyo_seq.size(), 2);
        Decision d[4] {};
        int s[4] {};
        Plan::iterateAvailablePlans(field, kumipuyo_seq, len, [&](const RefPlan& plan) {
            if (!plan.isRensaPlan())
                return;

            int enemy_score = gazeResult.estimateMaxScore(frame_id + plan.totalFrames(), enemy);
            bool update = false;
            if (plan.chains() <= 1 && plan.score() - enemy_score >= scoreForOjama(12)) {
                update = true;
            }
            if (plan.chains() <= 2 && plan.score() - enemy_score >= scoreForOjama(18) && plan.score() >= scoreForOjama(18)) {
                update = true;
            }
#if 1
            if (plan.chains() <= 3 && plan.score() - enemy_score >= scoreForOjama(18) && plan.score() >= scoreForOjama(21)) {
                update = true;
            }
#endif
#if 0
            if (plan.chains() <= 4 && enemy.field.countColorPuyos() <= 18) {
                if (plan.score() - enemy_score >= scoreForOjama(60) && enemy_score <= scoreForOjama(60)) {
                    update = true;
                }
            }
#endif

            if (!update)
                return;

            size_t size = plan.decisionSize();
            if (size > 3) { size = 3; }

            int diff_score = plan.score() - enemy_score;
            if (s[size] < diff_score) {
                s[size] = diff_score;
                d[size] = plan.firstDecision();
            }
        });

        for (size_t i = 0; i < 4; ++i) {
            if (d[i].isValid()) {
                return DropDecision(d[i], "TSUBUSHI\n" + gazeMessage(frame_id, me, enemy, gazeResult));
            }
        }
    }

    // TAIOU
#if 0
    if (true) {
        int len = std::min(kumipuyo_seq.size(), 3);
        Decision d[3] {};
        int s[3] {};
        Plan::iterateAvailablePlans(field, kumipuyo_seq, len, [&](const RefPlan& plan) {
                if (enemy.isRensaOngogin() && me.totalOjama(enemy) >= 3) {
                    if (plan.score() >= scoreForOjama(std::max(0, me.totalOjama(enemy) - 3))) {

                        sc_->addScore(STRATEGY_TAIOU, 1.0);
                        sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
                        return;
                    }
                }
            }
   }
#endif

    double beginTimeSec = currentTime();
    DropDecision dd = thinkByThinker(frame_id, field, kumipuyo_seq, me, enemy, fast);
    if (dd.isValid()) {
        double endTimeSec = currentTime();
        double durationSec = endTimeSec - beginTimeSec;

        std::stringstream ss;
        ss << (durationSec * 1000) << " [ms]";
        dd.setMessage(dd.message() + "\n" + gazeMessage(frame_id, me, enemy, gazeResult) + ss.str());
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
#if 0
    return rush_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, fast);
#endif

    const bool usesDecisionBook = true;
    const bool usesRensaHandTree = !fast;

    if (fast) {
        return pattern_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, gazer_.gazeResult(), fast,
                                       usesDecisionBook, usesRensaHandTree);
    }

    if (enemy.isRensaOngoing() || me.totalOjama(enemy) > 2) {
#if 0
        if (enemy.rensaFinishingFrameId() - frame_id > 60 * 4 && field.countPuyos() < 64) {
            return beam_thinker_->think(frame_id, field, kumipuyo_seq, me, enemy, fast);
        }
#endif
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

std::string YukinaAI::gazeMessage(int frame_id, const PlayerState& me, const PlayerState& enemy, const GazeResult& gazeResult) const
{
    std::stringstream ss;
    if (enemy.isRensaOngoing()) {
        ss << "Gazed (ongoing) : " << enemy.currentRensaResult.score
           << " in " << (enemy.rensaFinishingFrameId() - frame_id) << " / ";
    } else {
        ss << "Gazed (feasible) = "
           << gazeResult.estimateMaxFeasibleScore(frame_id + 100, enemy)
           << " in " << 100 << " / "
           << gazeResult.estimateMaxFeasibleScore(frame_id + 300, enemy)
           << " in " << 300 << " / "
           << gazeResult.estimateMaxFeasibleScore(frame_id + 500, enemy)
           << " in " << 500 << endl;
        ss << "Gazed (possible) = "
           << gazeResult.estimateMaxScore(frame_id + 100, enemy)
           << " in " << 100 << " / "
           << gazeResult.estimateMaxScore(frame_id + 300, enemy)
           << " in " << 300 << " / "
           << gazeResult.estimateMaxScore(frame_id + 500, enemy)
           << " in " << 500 << endl;
        }

    ss << "OJAMA: "
       << "fixed=" << me.fixedOjama << " "
       << "pending=" << me.pendingOjama << " "
       << "total=" << me.totalOjama(enemy) << " "
       << "frameId=" << me.rensaFinishingFrameId() << " / ";

    return ss.str();
}
