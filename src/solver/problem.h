#ifndef SOLVER_PROBLEM_H_
#define SOLVER_PROBLEM_H_

#include <map>
#include <set>
#include <string>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo_seq.h"

struct Problem {
    static Problem readProblem(const std::string& filename);

    std::string name;
    CoreField field[2];
    KumipuyoSeq kumipuyoSeq[2];
    std::set<Decision> answers;
};

#endif
