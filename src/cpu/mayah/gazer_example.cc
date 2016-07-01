#include "gazer.h"

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "solver/problem.h"

using namespace std;

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <problem>" << endl;
        return 1;
    }

    Problem problem = Problem::readProblem(argv[1]);

    Gazer gazer;
    gazer.initialize(10);
    gazer.gaze(10, problem.enemyState.field, problem.enemyState.seq);

    cout << gazer.gazeResult().toRensaInfoString() << endl;

    return 0;
}
