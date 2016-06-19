#include "yukina_ai.h"

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/probability/puyo_set_probability.h"
#include "core/probability/column_puyo_list_probability.h"

#include "yukina_ai.h"

using namespace std;

DECLARE_int32(num_threads);

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

    // initialize PuyoSetProbability and ColumnPuyoListProbability here.
    (void)PuyoSetProbability::instanceSlow();
    (void)ColumnPuyoListProbability::instanceSlow();

    LOG(INFO) << "num_threads = " << FLAGS_num_threads;

    YukinaAI(argc, argv).runLoop();
    return 0;
}
