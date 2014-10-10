#include "mayah_ai.h"

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/client/ai/endless/endless.h"
#include "core/sequence_generator.h"

using namespace std;

DEFINE_bool(show_field, false, "show field after each hand");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    Endless endless(std::unique_ptr<AI>(new MayahAI(argc, argv)));
    endless.setVerbose(FLAGS_show_field);

    KumipuyoSeq seq = generateSequence();
    int score = endless.run(seq);

    cout << "score=" << score << endl;
    cout << "seq=" << seq.toString() << endl;

    return 0;
}
