#include "cpu.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

int q_t = 1;
int w_t = 1;
int e_t = 1;
int r_t = 1;
int t_t = 1;
int y_t = 3;
int u_t = 1;
int i_t = 0;
int o_t = 0;
int p_t = 4;
int a_t = 1;

int takasa_point = 240; // G,S
int ruiseki_point = 0; // T,S
int renketu_bairitu = 4; // S
bool is_2dub_cpu = true;

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TestLockitAI().runLoop();
    return 0;
}
