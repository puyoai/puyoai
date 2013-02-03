#include <gflags/gflags.h>
#include <glog/logging.h>

#include "ai_manager.h"
#include "puyo_possibility.h"

DEFINE_int32(thread, 1, "Limit the maximum number of threads");

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::ParseCommandLineFlags(&argc, &argv, true);
    TsumoPossibility::initialize();

    return AIManager(argv[1]).runLoop();
}
