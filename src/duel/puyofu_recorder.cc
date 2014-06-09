#include "duel/puyofu_recorder.h"

#include <glog/logging.h>

#include "core/state.h"
#include "duel/field_realtime.h"
#include "duel/game_state.h"

// TODO(mayah): Use PlainField instead of CoreField here.

using namespace std;

void PuyofuRecorder::newGameWillStart()
{
}

void PuyofuRecorder::onUpdate(const GameState& gameState)
{
    // TODO(mayah); When should we log here?

    for (int pi = 0; pi < 2; ++pi) {
        const FieldRealtime& field = gameState.field(pi);

        // TODO(mayah): Why not passing usec here instead of passing 0?
        // Or, do we really need usec?
        if (field.userState().grounded)
            setField(pi, field.field(), field.kumipuyoSeq(), 0);
    }
}

void PuyofuRecorder::gameHasDone()
{
    FILE* fp;

    fp = fopen("/tmp/puyoai_1p.txt", "a");
    if (fp) {
        emitFieldTransitionLog(fp, 0);
        fprintf(fp, "=== end ===\n");
        fclose(fp);
    }

    fp = fopen("/tmp/puyoai_2p.txt", "a");
    if (fp) {
        emitFieldTransitionLog(fp, 1);
        fprintf(fp, "=== end ===\n");
        fclose(fp);
    }

    clear();
}

template <class Field>
void GetRensimQueryString(const Field& f, string* out)
{
    bool started = false;
    for (int y = 13; y >= 1; y--) {
        for (int x = 1; x <= 6; x++) {
            PuyoColor c = f.color(x, y);
            if (c != PuyoColor::EMPTY) {
                started = true;
            }
            if (started) {
                out->push_back(static_cast<int>(c) + '0');
            }
        }
    }
}

void PuyofuRecorder::setField(int pi, const CoreField& field, const KumipuyoSeq& kumipuyoSeq, int time)
{
    moves_.push_back(unique_ptr<Move>(new Move(pi, field, kumipuyoSeq, time)));
}

void PuyofuRecorder::emitFieldTransitionLog(FILE* fp, int pi) const
{
    CoreField field;
    for (const auto& m : moves_) {
        if (m->pi != pi)
            continue;

        string before, after;
        GetRensimQueryString(field, &before);
        GetRensimQueryString(m->field, &after);
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
