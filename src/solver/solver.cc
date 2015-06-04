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
    req.playerFrameRequest[0].field = problem.myState.field.toPlainField();
    req.playerFrameRequest[1].field = problem.enemyState.field.toPlainField();
    req.playerFrameRequest[0].kumipuyoSeq = problem.myState.seq;
    req.playerFrameRequest[1].kumipuyoSeq = problem.enemyState.seq;

    // TODO(mayah): Consider adding an interface to set state.
    ai_->me_ = problem.myState;
    ai_->enemy_ = problem.enemyState;

    ai_->gaze(req.frameId, CoreField(problem.enemyState.field), problem.enemyState.seq);

    DropDecision dd = ai_->think(3,
                                 problem.myState.field,
                                 problem.myState.seq,
                                 ai_->myPlayerState(),
                                 ai_->enemyPlayerState(),
                                 false);
    return dd.decision();
}
