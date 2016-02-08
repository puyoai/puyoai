#include "solver/problem.h"

#include <fstream>
#include <string>

#include <glog/logging.h>

#include "base/strings.h"

using namespace std;

bool parsePlayerState(const toml::Value& value, PlayerState* state)
{
    {
        const toml::Value* field = value.find("field");
        CHECK(field && field->is<toml::Array>());
        string s;
        for (const auto& v : field->as<toml::Array>())
            s += v.as<string>();
        state->field = CoreField(s);
    }
    {
        const toml::Value* next = value.find("next");
        CHECK(next && next->is<string>());
        state->seq = KumipuyoSeq(next->as<string>());
    }

    return true;
}

Problem Problem::readProblem(const string& filename)
{
    ifstream ifs(filename);
    toml::ParseResult result = toml::parse(ifs);
    if (!result.valid()) {
        CHECK(false) << result.errorReason;
        return Problem();
    }

    return parse(result.value);
}

Problem Problem::parse(const toml::Value& value)
{
    Problem problem;

    {
        const toml::Value* name = value.find("name");
        CHECK(name && name->is<string>());
        problem.name = name->as<string>();
    }

    {
        const toml::Value* me = value.find("me");
        CHECK(me && me->is<toml::Table>());
        CHECK(parsePlayerState(*me, &problem.myState));
    }
    {
        const toml::Value* enemy = value.find("enemy");
        CHECK(enemy && enemy->is<toml::Table>());
        CHECK(parsePlayerState(*enemy, &problem.enemyState));
    }
    {
        const toml::Value* answers = value.find("answer.answers");
        CHECK(answers && answers->is<toml::Array>());
        for (const auto& decisions : answers->as<toml::Array>()) {
            CHECK(decisions.is<toml::Array>());
            CHECK_EQ(decisions.size(), 2UL);
            int x = decisions.get<int>(0);
            int r = decisions.get<int>(1);
            problem.answers.insert(Decision(x, r));
        }
    }

    return problem;
}
