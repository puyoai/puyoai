#ifndef DUEL_PUYOFU_RECORDER_H_
#define DUEL_PUYOFU_RECORDER_H_

#include <stdio.h>

#include <memory>
#include <utility>
#include <vector>

#include "core/core_field.h"
#include "core/kumipuyo_seq.h"
#include "core/server/game_state_observer.h"

class CoreField;

class PuyofuRecorder : public GameStateObserver {
public:
    enum class Mode {
        TRANSITION_LOG,
        FIELD_LOG
    };

    virtual ~PuyofuRecorder() {}

    virtual void newGameWillStart() override;
    virtual void onUpdate(const GameState&) override;
    virtual void gameHasDone(GameResult) override;

    void setMode(Mode mode) { mode_ = mode; }

private:
    void addMove(int pi, const PlainField&, const KumipuyoSeq&, int time);

    void emitLog(FILE* fp, int pi) const;
    void emitTransitionLog(FILE* fp, int pi) const;
    void emitFieldLog(FILE* fp, int pi) const;

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

    Mode mode_ = Mode::TRANSITION_LOG;
    std::vector<std::unique_ptr<Move>> moves_;
};

#endif
