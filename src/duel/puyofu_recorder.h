#ifndef DUEL_PUYOFU_RECORDER_H_
#define DUEL_PUYOFU_RECORDER_H_

#include <stdio.h>

#include <memory>
#include <utility>
#include <vector>

#include "core/field/core_field.h"
#include "core/kumipuyo.h"
#include "duel/game_state_observer.h"

class CoreField;

class PuyofuRecorder : public GameStateObserver {
public:
    virtual ~PuyofuRecorder() {}

    virtual void newGameWillStart() OVERRIDE;
    virtual void onUpdate(const GameState&) OVERRIDE;
    virtual void gameHasDone() OVERRIDE;

private:
    void addMove(int pi, const CoreField&, const KumipuyoSeq&, int time);

    void emitFieldTransitionLog(FILE* fp, int pi) const;

    bool clear() const { return moves_.empty(); }

    struct Move {
        Move(int pi, const PlainField& field, const KumipuyoSeq& kumipuyoSeq, int time) :
            pi(pi), field(field), kumipuyoSeq(kumipuyoSeq), time(time)
        {
        }

        int pi;
        PlainField field;
        KumipuyoSeq kumipuyoSeq;
        int time;
    };

    std::vector<std::unique_ptr<Move>> moves_;
};

#endif
