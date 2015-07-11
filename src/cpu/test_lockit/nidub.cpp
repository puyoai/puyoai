#include "cpu.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    struct test_lockit::cpu::Configuration config;
    config.q_t = 1;
    config.w_t = 1;
    config.e_t = 1;
    config.r_t = 1;
    config.t_t = 1;
    config.y_t = 3;
    config.u_t = 1;
    config.i_t = 0;
    config.o_t = 0;
    config.p_t = 4;
    config.a_t = 1;

    config.takasa_point = 240;
    config.ruiseki_point = 0;
    config.renketu_bairitu = 4;
    config.is_2dub_cpu = true;
    config.uses_2x_hyouka = false;

    test_lockit::TestLockitAI(config).runLoop();

    return 0;
}
