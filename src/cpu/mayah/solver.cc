#include "mayah_ai.h"

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/problem/solver.h"

using namespace std;

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <problem>" << endl;
        return 1;
    }

    Solver solver(std::unique_ptr<AI>(new MayahAI(argc, argv)));
    solver.solve(argv[1]);

    return 0;
}
