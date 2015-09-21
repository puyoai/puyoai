#include "mayah_ai.h"

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/probability/puyo_set_probability.h"
#include "solver/solver.h"

using namespace std;

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <problem>" << endl;
        return 1;
    }

    Solver solver(std::unique_ptr<AI>(new MayahAI(argc, argv)));
    Problem problem = Problem::readProblem(argv[1]);

    Decision d = solver.solve(problem);
    cout << problem.name << ": " << (problem.answers.count(d) ? "OK" : "NG") << endl;
    if (!problem.answers.count(d)) {
        cout << "  Your decision: " << d.toString() << endl;
        cout << "  Answers: ";
        for (const auto& d : problem.answers) {
            cout << d.toString();
        }
        cout << endl;
    }

    return 0;
}
