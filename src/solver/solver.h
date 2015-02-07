#ifndef SOLVER_SOLVER_H_
#define SOLVER_SOLVER_H_

#include <string>
#include <memory>

#include "core/client/ai/ai.h"
#include "solver/problem.h"

class Solver {
public:
    explicit Solver(std::unique_ptr<AI> ai);

    Decision solve(const Problem&);

private:
    std::unique_ptr<AI> ai_;
};

#endif
