#include "cpu.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

namespace test_lockit {

int q_t = 1;
int w_t = 1;
int e_t = 0;
int r_t = 1;
int t_t = 1;
int y_t = 2;
int u_t = 1;
int i_t = 0;
int o_t = 1;
int p_t = 2;
int a_t = 1;

int takasa_point = 240;
int ruiseki_point = 6;
int renketu_bairitu = 1;
bool is_2dub_cpu = false;
bool uses_2x_hyouka = true;

}  // namespace test_lockit

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    test_lockit::TestLockitAI().runLoop();

    return 0;
}
