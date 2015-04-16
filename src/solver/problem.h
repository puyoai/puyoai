#ifndef SOLVER_PROBLEM_H_
#define SOLVER_PROBLEM_H_

#include <map>
#include <set>
#include <string>

#include <toml/toml.h>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo_seq.h"
#include "core/player_state.h"

struct Problem {
    static Problem readProblem(const std::string& filename);
    static Problem parse(const toml::Value&);

    std::string name;
    PlayerState myState;
    EnemyState enemyState;
    std::set<Decision> answers;
};

#endif
