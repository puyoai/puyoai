#ifndef CORE_PROBLEM_PROBLEM_H_
#define CORE_PROBLEM_PROBLEM_H_

#include <map>
#include <string>
#include <vector>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo_seq.h"

struct Problem {
public:
    static Problem readProblem(const std::string& filename);

    std::string name;
    int hand;
    std::vector<Decision> enemyHands;
    CoreField field[2];
    KumipuyoSeq kumipuyoSeq[2];
    std::map<std::vector<Decision>, int> answers;
};

#endif
