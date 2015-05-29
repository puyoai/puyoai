#ifndef CPU_TEST_LOCKIT_READ_H_
#define CPU_TEST_LOCKIT_READ_H_

#include "lockit_constant.h"

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

    int field[6][TATE];
    int yosou_field[6][TATE];
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

    int saiki(int[][TATE], int[][12], int, int, int*, int);
    int saiki_right(int[][TATE], int[][12], int, int, int*, int);
    int saiki_left(int[][TATE], int[][12], int, int, int*, int);
    int saiki_up(int[][TATE], int[][12], int, int, int*, int);
    int saiki_down(int[][TATE], int[][12], int, int, int*, int);
    int syou(int[][TATE], int, int, int, int[]);
    int syou_right(int[][TATE], int, int, int, int[]);
    int syou_left(int[][TATE], int, int, int, int[]);
    int syou_up(int[][TATE], int, int, int, int[]);
    int syou_down(int[][TATE], int, int, int, int[]);

    int field12[6];
    int field_hoz[6][TATE];
    int act_on_1st;
    int rensa_end_once;
    int setti_basyo[4];
};

#endif
