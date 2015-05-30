#include "duel/puyofu_recorder.h"

#include <glog/logging.h>

#include "core/server/game_state.h"
#include "duel/field_realtime.h"

// TODO(mayah): Use PlainField instead of CoreField here.

using namespace std;

void PuyofuRecorder::newGameWillStart()
{
}

void PuyofuRecorder::onUpdate(const GameState& gameState)
{
    // TODO(mayah); When should we log here?

    for (int pi = 0; pi < 2; ++pi) {
        const PlayerGameState& pgs = gameState.playerGameState(pi);

        // TODO(mayah): Why not passing usec here instead of passing 0?
        // Or, do we really need usec?
        if (pgs.event.grounded) {
            KumipuyoSeq seq = pgs.kumipuyoSeq;
            if (seq.size() >= 4) {
                seq.resize(3);
            }

            PlainField pf = pgs.field;
            pf.drop();
            addMove(pi, pf, seq, 0);
        }
    }
}

void PuyofuRecorder::gameHasDone(GameResult)
{
    FILE* fp;

    fp = fopen("/tmp/puyoai_1p.txt", "a");
    if (fp) {
        emitLog(fp, 0);
        fprintf(fp, "=== end ===\n");
        fclose(fp);
    }

    fp = fopen("/tmp/puyoai_2p.txt", "a");
    if (fp) {
        emitLog(fp, 1);
        fprintf(fp, "=== end ===\n");
        fclose(fp);
    }

    clear();
}

void PuyofuRecorder::addMove(int pi, const PlainField& field, const KumipuyoSeq& kumipuyoSeq, int time)
{
    moves_.push_back(unique_ptr<Move>(new Move(pi, field, kumipuyoSeq, time)));
}

void PuyofuRecorder::emitLog(FILE* fp, int pi) const
{
    switch (mode_) {
    case Mode::TRANSITION_LOG:
        emitTransitionLog(fp, pi);
        break;
    case Mode::FIELD_LOG:
        emitFieldLog(fp, pi);
        break;
    default:
        CHECK(false) << "Unknown mode: " << static_cast<int>(mode_) << endl;
    }
}

void PuyofuRecorder::emitTransitionLog(FILE* fp, int pi) const
{
    PlainField field;
    for (const auto& m : moves_) {
        if (m->pi != pi)
            continue;

        string before = field.toString('0');
        string after = m->field.toString('0');

        if (before.empty()) {
            if (after.empty())
                continue;
            before.push_back('0');
        }
        if (after.empty())
            after.push_back('0');
        fprintf(fp, "%s %s %s\n", before.c_str(), m->kumipuyoSeq.toString().c_str(), after.c_str());
        field = m->field;
    }
}

void PuyofuRecorder::emitFieldLog(FILE* fp, int pi) const
{
    for (const auto& m : moves_) {
        if (m->pi != pi)
            continue;

        CoreField cf(m->field);
        fprintf(fp, "%s\n%s\n", cf.toDebugString().c_str(), m->kumipuyoSeq.toString().c_str());
    }
}
