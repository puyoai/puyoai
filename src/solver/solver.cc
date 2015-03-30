#include "solver/solver.h"

#include <iostream>

#include "core/frame_request.h"

using namespace std;

Solver::Solver(unique_ptr<AI> ai) :
    ai_(move(ai))
{
}

Decision Solver::solve(const Problem& problem)
{
    FrameRequest req;
    req.frameId = 1;
    ai_->gameWillBegin(req);

    req.frameId = 2;
    req.playerFrameRequest[0].field = problem.mySituation.field;
    req.playerFrameRequest[1].field = problem.enemySituation.field;
    req.playerFrameRequest[0].kumipuyoSeq = problem.mySituation.kumipuyoSeq;
    req.playerFrameRequest[1].kumipuyoSeq = problem.enemySituation.kumipuyoSeq;

    // TODO(mayah): Consider adding an interface to set situation.
    ai_->me_.field = problem.mySituation.field;
    ai_->me_.seq = problem.mySituation.kumipuyoSeq;
    ai_->enemy_.field = problem.enemySituation.field;
    ai_->enemy_.seq = problem.enemySituation.kumipuyoSeq;

    ai_->gaze(req.frameId, CoreField(problem.enemySituation.field), problem.enemySituation.kumipuyoSeq);

    DropDecision dd = ai_->think(3,
                                 problem.mySituation.field,
                                 problem.mySituation.kumipuyoSeq,
                                 ai_->myPlayerState(),
                                 ai_->enemyPlayerState(),
                                 false);
    return dd.decision();
}
