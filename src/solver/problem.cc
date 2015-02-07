#include "solver/problem.h"

#include <fstream>
#include <string>

#include <glog/logging.h>
#include <toml/toml.h>

#include "base/strings.h"

using namespace std;

Problem Problem::readProblem(const string& filename)
{
    Problem problem;

    toml::Value value;
    try {
        ifstream ifs(filename);
        toml::Parser parser(ifs);
        value = parser.parse();
    } catch (std::exception& e) {
        CHECK(false) << e.what();
        return problem;
    }

    CHECK(value.valid());

    {
        const toml::Value* name = value.find("name");
        CHECK(name && name->is<string>());
        problem.name = name->as<string>();
    }

    {
        const toml::Value* field = value.find("field");
        CHECK(field && field->is<toml::Array>());
        string s;
        for (const auto& v : field->as<toml::Array>())
            s += v.as<string>();
        problem.field[0] = CoreField(s);
    }
    {
        const toml::Value* next = value.find("next");
        CHECK(next && next->is<string>());
        problem.kumipuyoSeq[0] = KumipuyoSeq(next->as<string>());
    }

    {
        const toml::Value* enemyField = value.find("enemy_field");
        CHECK(enemyField && enemyField->is<toml::Array>());
        string s;
        for (const auto& v : enemyField->as<toml::Array>())
            s += v.as<string>();
    }
    {
        const toml::Value* enemyNext = value.find("enemy_next");
        CHECK(enemyNext && enemyNext->is<string>());
        problem.kumipuyoSeq[1] = KumipuyoSeq(enemyNext->as<string>());
    }
    {
        const toml::Value* enemyDecisions = value.find("enemy_decisions");
        CHECK(enemyDecisions && enemyDecisions->is<toml::Array>());
        for (const auto& vs : enemyDecisions->as<toml::Array>()) {
            CHECK(vs.size() == 2UL);
            int x = vs.get<int>(0);
            int r = vs.get<int>(1);
            problem.enemyHands.push_back(Decision(x, r));
        }
    }

    {
        const toml::Value* answers = value.find("answers");
        CHECK(answers && answers->is<toml::Array>());
        for (const auto& vs : answers->as<toml::Array>()) {
            int score = vs.get<int>("score");
            const toml::Array& decisions = vs.get<toml::Array>("decisions");
            CHECK(decisions.size() == 2UL);
            int x1 = decisions[0].get<int>(0);
            int r1 = decisions[0].get<int>(1);
            int x2 = decisions[1].get<int>(0);
            int r2 = decisions[1].get<int>(1);
            vector<Decision> ds {
                Decision(x1, r1),
                Decision(x2, r2)
            };
            problem.answers[ds] = score;
        }
    }

    return problem;
}
