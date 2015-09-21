#include "mayah_ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

#include "base/file.h"
#include "base/strings.h"
#include "core/probability/puyo_set_probability.h"
#include "solver/problem.h"
#include "solver/solver.h"

using namespace std;

static unique_ptr<DebuggableMayahAI> makeAI(Executor* executor = nullptr)
{
    int argc = 1;
    char arg[] = "mayah";
    char* argv[] = {arg};
    return unique_ptr<DebuggableMayahAI>(new DebuggableMayahAI(argc, argv, executor));
}

static vector<std::string> listTestcases()
{
    const string problemDir = file::joinPath(SRC_DIR, "cpu", "mayah", "situation_tests");

    vector<string> files;
    if (!file::listFiles(problemDir, &files)) {
        return vector<string>();
    }

    vector<string> testcases;
    for (const auto& f : files) {
        if (!strings::hasSuffix(f, ".toml"))
            continue;

        testcases.push_back(file::joinPath(problemDir, f));
    }

    return testcases;
}

class MayahAISituationTest : public testing::TestWithParam<string> {};

TEST_P(MayahAISituationTest, ThinkAsExpected) {
    const string& filename = GetParam();
    cout << filename << endl;

    Solver solver(makeAI());
    Problem problem = Problem::readProblem(filename);

    Decision d = solver.solve(problem);
    EXPECT_TRUE(problem.answers.count(d))
        << "  Problem name : " << problem.name << endl
        << "  Your decision: " << d.toString();
}

INSTANTIATE_TEST_CASE_P(
    SituationTestCases,
    MayahAISituationTest,
    testing::ValuesIn(listTestcases()));

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    google::ParseCommandLineFlags(&argc, &argv, true);

    return RUN_ALL_TESTS();
}
