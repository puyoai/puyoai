#ifndef CPU_TEST_LOCKIT_FIELD_H_
#define CPU_TEST_LOCKIT_FIELD_H_

#include "color.h"
#include "lockit_constant.h"

namespace test_lockit {

struct TLRensaResult;

bool isTLFieldEmpty(const PuyoColor field[6][kHeight]);
int countNormalColor13(const PuyoColor f[][kHeight]);
void copyField(const PuyoColor src[][kHeight], PuyoColor dst[][kHeight]);

// simulates a 連鎖 and returns its result.  The argument |field| will be updated
// to be state of field after the 連鎖.
TLRensaResult simulate(PuyoColor field[][kHeight]);

// --------------------------------------------------------------------

void saiki(const PuyoColor field[][kHeight], int point[][12], int x, int y, int* num, PuyoColor incol);
void saiki_3(const PuyoColor ba[][kHeight], int point[][12], int x, int y, int* num, PuyoColor incol);
void saiki_4(PuyoColor ba[][kHeight], int x, int y, int* num, PuyoColor incol);

void syou(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[]);
void syou_downx(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[], int* num);

bool setti_puyo(PuyoColor ba[][kHeight], int aa, PuyoColor nx1, PuyoColor nx2, int setti_basyo[]);
int setti_puyo_1(PuyoColor ba[][kHeight], int eex, PuyoColor eecol);

int chousei_syoukyo(PuyoColor ba[][kHeight], int setti_basyo[]);
int chousei_syoukyo_2(PuyoColor ba[][kHeight], int setti_basyo[], int* chain, int dabuchk[], int* ichiren_kesi, int* score);
int chousei_syoukyo_3(PuyoColor bass[][kHeight], int[], int* poi2s, int* score, int tokus, int i2, int j2, int ruiseki_point);
int chousei_syoukyo_sc(PuyoColor ba[][kHeight], int setti_basyo[], int* score);
int hon_syoukyo(PuyoColor ba[][kHeight]);
int hon_syoukyo_score(PuyoColor ba[][kHeight], int* score, int* quick);

void setti_ojama(PuyoColor f[][kHeight], int numOjama);

}  // namespace test_lockit

#endif  // CPU_TEST_LOCKIT_FIELD_H_
