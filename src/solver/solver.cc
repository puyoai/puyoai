#include "solver/solver.h"

#include <iostream>

#include "core/frame_request.h"
#include "solver/problem.h"

using namespace std;

Solver::Solver(unique_ptr<AI> ai) :
    ai_(move(ai))
{
}

int Solver::solve(const std::string& filename)
{
    Problem problem = Problem::readProblem(filename);

    FrameRequest req;
    req.frameId = 1;
    ai_->gameWillBegin(req);

    req.frameId = 2;
    req.playerFrameRequest[0].field = problem.field[0];
    req.playerFrameRequest[1].field = problem.field[1];
    req.playerFrameRequest[0].kumipuyoSeq = problem.kumipuyoSeq[0];
    req.playerFrameRequest[1].kumipuyoSeq = problem.kumipuyoSeq[1];

    ai_->gaze(req.frameId, CoreField(problem.field[1]), problem.kumipuyoSeq[1]);

    DropDecision d0;
    {
        AdditionalThoughtInfo info { ai_->myPlayerState(), ai_->enemyPlayerState() };
        d0 = ai_->think(3, problem.field[0], problem.kumipuyoSeq[0], info, false);
    }

    problem.field[0].dropKumipuyo(d0.decision(), problem.kumipuyoSeq[0].front());
    problem.kumipuyoSeq[0].dropFront();
    problem.field[0].simulate();

    DropDecision d1;
    {
        AdditionalThoughtInfo info { ai_->myPlayerState(), ai_->enemyPlayerState() };
        d1 = ai_->think(4, problem.field[0], problem.kumipuyoSeq[0], info, false);
    }

    cout << problem.name << ": "
         << d0.decision().toString() << "-"
         << d1.decision().toString() << endl;

    vector<Decision> decisions { d0.decision(), d1.decision() };
    cout << "score: " << problem.answers[decisions] << endl;

    return problem.answers[decisions];
}
