#include <gflags/gflags.h>
#include <glog/logging.h>

#include "ai_manager.h"
#include "puyo_possibility.h"

DEFINE_int32(thread, 1, "Limit the maximum number of threads");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    LOG(INFO) << "initializede";

    return AIManager(argv[1]).runLoop();
}
