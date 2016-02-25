#ifndef CPU_TEST_LOCKIT_READ_H_
#define CPU_TEST_LOCKIT_READ_H_

#include "core/core_field.h"
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

    CoreField field;
    CoreField yosou_field;
    PuyoColor tsumo[6];
    int set_puyo_once;
    int score;
    int keep_score;
    int zenkesi;
    int id;
    int te_x;
    int te_r;

private:
    PuyoColor field12[6];
    CoreField field_hoz;
    int act_on_1st;
    int rensa_end_once;
    int setti_basyo[4];
};

}  // namespace test_lockit

#endif
