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
    req.playerFrameRequest[0].field = problem.field[0];
    req.playerFrameRequest[1].field = problem.field[1];
    req.playerFrameRequest[0].kumipuyoSeq = problem.kumipuyoSeq[0];
    req.playerFrameRequest[1].kumipuyoSeq = problem.kumipuyoSeq[1];

    // TODO(mayah): Consider adding an interface to set situation.
    ai_->me_.field = problem.field[0];
    ai_->me_.seq = problem.kumipuyoSeq[0];
    ai_->enemy_.field = problem.field[1];
    ai_->enemy_.seq = problem.kumipuyoSeq[1];

    ai_->gaze(req.frameId, CoreField(problem.field[1]), problem.kumipuyoSeq[1]);

    DropDecision dd = ai_->think(3,
                                 problem.field[0],
                                 problem.kumipuyoSeq[0],
                                 ai_->myPlayerState(),
                                 ai_->enemyPlayerState(),
                                 false);
    return dd.decision();
}
