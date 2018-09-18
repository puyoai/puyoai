#ifndef CPU_TEST_LOCKIT_CPU_CONFIGURATION_H_
#define CPU_TEST_LOCKIT_CPU_CONFIGURATION_H_

namespace test_lockit {
namespace cpu {

struct Configuration {
    int q_t;
    int w_t;
    int e_t;
    int r_t;
    int t_t;
    int y_t;
    int u_t;
    int i_t;
    int o_t;
    int p_t;
    int a_t;

    int takasa_point;
    int ruiseki_point;
    int renketu_bairitu;

    bool is_2dub_cpu;
    bool uses_2x_hyouka;
};

/* for TAIOU TYPE?
        int q_t=1;
        int w_t=1;
        int e_t=0;
        int r_t=1;
        int t_t=1;
        int y_t=3;
        int u_t=1;
        int i_t=0;
        int o_t=0;
        int p_t=3;
        int a_t=1;*/

}  // namespace cpu
}  // namespace test_lockit

#endif  // CPU_TEST_LOCKIT_CPU_CONFIGURATION_H_
