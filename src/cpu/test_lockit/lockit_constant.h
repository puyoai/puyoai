#ifndef CPU_TEST_LOCKIT_CONSTANT_H_
#define CPU_TEST_LOCKIT_CONSTANT_H_

namespace test_lockit {

const int kHeight = 16;
const int EE_SIZE = 1;

const int TM_TMNMUM = 40;
const int TM_COLNUM = 20;
const int TM_COLPER = 8;
const int TM_KINPT = 30;
const int TM_JAKKINPT = 10;
const int TM_TANKINPT = 10;
const int TM_OBJE = 10;

extern int q_t;
extern int w_t;
extern int e_t;
extern int r_t;
extern int t_t;
extern int y_t;
extern int u_t;
extern int i_t;
extern int o_t;
extern int p_t;
extern int a_t;

extern int takasa_point;
extern int ruiseki_point;
extern int renketu_bairitu;

extern bool is_2dub_cpu;
extern bool uses_2x_hyouka;

}  // namespace test_lockit

#endif
