#ifndef CPU_TEST_LOCKIT_READ_H_
#define CPU_TEST_LOCKIT_READ_H_

#include "core/puyo_color.h"
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

    PuyoColor field[6][kHeight];
    PuyoColor yosou_field[6][kHeight];
    PuyoColor tsumo[6];
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

    PuyoColor field12[6];
    PuyoColor field_hoz[6][kHeight];
    int act_on_1st;
    int rensa_end_once;
    int setti_basyo[4];
};

} // namespace test_lockit

#endif
