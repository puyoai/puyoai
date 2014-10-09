#include "mayah_ai.h"

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/client/ai/endless/endless.h"
#include "core/sequence_generator.h"

using namespace std;

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    Endless endless(std::unique_ptr<AI>(new MayahAI(argc, argv)));

    KumipuyoSeq seq = generateSequence();
    cout << endless.run(seq) << endl;

    return 0;
}
