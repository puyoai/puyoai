#ifndef SOLVER_SOLVER_H_
#define SOLVER_SOLVER_H_

#include <string>
#include <memory>

#include "core/client/ai/ai.h"
#include "solver/problem.h"

class Solver {
public:
    explicit Solver(std::unique_ptr<AI> ai);

    // Returns true if |ai| could return one of the answer decisions.
    // Otherwise, false will be returned.
    bool solve(const Problem&);

private:
    std::unique_ptr<AI> ai_;
};

#endif
