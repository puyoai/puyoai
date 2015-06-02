#ifndef CPU_TEST_LOCKIT_READ_H_
#define CPU_TEST_LOCKIT_READ_H_

#include "lockit_constant.h"

namespace test_lockit {

class READ_P {
public:
    READ_P();
    ~READ_P();

    void ref();
    void fall();
    int setti_puyo();
    void setti_12();
    void field_kioku();
    int field_hikaku();

    int field[6][kHeight];
    int yosou_field[6][kHeight];
    int tsumo[6];
    int act_on;
    int nex_on;
    int set_puyo;
    int set_puyo_once;
    int rensa_end;
    int score;
    int keep_score;
    int zenkesi;
    int id;
    int te_x;
    int te_r;

private:
    int chousei_syoukyo();

    int saiki(int[][kHeight], int[][12], int, int, int*, int);
    int saiki_right(int[][kHeight], int[][12], int, int, int*, int);
    int saiki_left(int[][kHeight], int[][12], int, int, int*, int);
    int saiki_up(int[][kHeight], int[][12], int, int, int*, int);
    int saiki_down(int[][kHeight], int[][12], int, int, int*, int);
    int syou(int[][kHeight], int, int, int, int[]);
    int syou_right(int[][kHeight], int, int, int, int[]);
    int syou_left(int[][kHeight], int, int, int, int[]);
    int syou_up(int[][kHeight], int, int, int, int[]);
    int syou_down(int[][kHeight], int, int, int, int[]);

    int field12[6];
    int field_hoz[6][kHeight];
    int act_on_1st;
    int rensa_end_once;
    int setti_basyo[4];
};

}  // namespace test_lockit

#endif
