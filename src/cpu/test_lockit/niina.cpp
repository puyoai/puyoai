#include "cpu.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

DEFINE_string(type, "nidub", "Type of niina. nidub, rendaS9, rendaGS9, or rendaGS9a, is valid.");

test_lockit::cpu::Configuration makeRendaGS9Configuration()
{
    struct test_lockit::cpu::Configuration config;
    config.q_t = 1;
    config.w_t = 1;
    config.e_t = 0;
    config.r_t = 1;
    config.t_t = 1;
    config.y_t = 2;
    config.u_t = 1;
    config.i_t = 0;
    config.o_t = 1;
    config.p_t = 2;
    config.a_t = 1;

    config.takasa_point = 240;
    config.ruiseki_point = 6;
    config.renketu_bairitu = 1;
    config.is_2dub_cpu = false;
    config.uses_2x_hyouka = false;

    return config;
}

test_lockit::cpu::Configuration makeRendaGS9aConfiguration()
{
    struct test_lockit::cpu::Configuration config;
    config.q_t = 1;
    config.w_t = 1;
    config.e_t = 0;
    config.r_t = 1;
    config.t_t = 1;
    config.y_t = 2;
    config.u_t = 1;
    config.i_t = 0;
    config.o_t = 1;
    config.p_t = 2;
    config.a_t = 1;

    config.takasa_point = 240;
    config.ruiseki_point = 6;
    config.renketu_bairitu = 1;
    config.is_2dub_cpu = false;
    config.uses_2x_hyouka = true;

    return config;
}

test_lockit::cpu::Configuration makeRendaS9Configuration()
{
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
    config.is_2dub_cpu = false;
    config.uses_2x_hyouka = false;

    return config;
}

test_lockit::cpu::Configuration makeNidubConfiguration()
{
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

    return config;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

    test_lockit::cpu::Configuration config;
    if (FLAGS_type == "rendaGS9") {
        config = makeRendaGS9Configuration();
    } else if (FLAGS_type == "rendaGS9a") {
        config = makeRendaGS9aConfiguration();
    } else if (FLAGS_type == "rendaS9") {
        config = makeRendaS9Configuration();
    } else if (FLAGS_type == "nidub") {
        config = makeNidubConfiguration();
    } else {
        CHECK(false) << "Unknown type: " << FLAGS_type;
    }

    test_lockit::TestLockitAI(config).runLoop();

    return 0;
}
