#include "core/problem/problem.h"

#include <fstream>
#include <glog/logging.h>

using namespace std;

Problem Problem::readProblem(const string& filename)
{
    ifstream ifs(filename);
    string str;

    Problem problem;

    CHECK(getline(ifs, str));
    CHECK_EQ(str, "FIELD:");

    string field1, field2;
    while (ifs >> str) {
        if (str == "NEXT:")
            break;
        field1 += str;
        ifs >> str;
        field2 += str;
    }

    problem.field[0] = CoreField(field1);
    problem.field[1] = CoreField(field2);

    CHECK(ifs >> str);
    problem.kumipuyoSeq[0] = KumipuyoSeq(str);
    CHECK(ifs >> str);
    problem.kumipuyoSeq[1] = KumipuyoSeq(str);

    CHECK(ifs >> str);
    CHECK_EQ(str, "ANSWERS:");

    int score, x1, r1, x2, r2;
    while (ifs >> score >> x1 >> r1 >> x2 >> r2) {
        vector<Decision> ds {
            Decision(x1, r1),
            Decision(x2, r2)
        };
        problem.answers[ds] = score;
    }

    return problem;
}
