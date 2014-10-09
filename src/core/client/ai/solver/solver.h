#ifndef CORE_CLIENT_AI_SOLVER_SOLVER_H_
#define CORE_CLIENT_AI_SOLVER_SOLVER_H_

#include <string>
#include <memory>

#include "core/client/ai/ai.h"

class Solver {
public:
    explicit Solver(std::unique_ptr<AI> ai);

    int solve(const std::string& filename);

private:
    std::unique_ptr<AI> ai_;
};

#endif
