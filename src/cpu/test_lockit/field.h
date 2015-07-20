#ifndef CPU_TEST_LOCKIT_FIELD_H_
#define CPU_TEST_LOCKIT_FIELD_H_

#include "color.h"
#include "lockit_constant.h"

namespace test_lockit {

struct TLRensaResult;

bool isTLFieldEmpty(const TLColor field[6][kHeight]);
int countNormalColor13(const TLColor f[][kHeight]);
void copyField(const TLColor src[][kHeight], TLColor dst[][kHeight]);

// simulates a 連鎖 and returns its result.  The argument |field| will be updated
// to be state of field after the 連鎖.
TLRensaResult simulate(TLColor field[][kHeight]);

// --------------------------------------------------------------------

void saiki(const TLColor field[][kHeight], int point[][12], int x, int y, int* num, TLColor incol);
void saiki_3(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol);
void saiki_4(TLColor ba[][kHeight], int x, int y, int* num, TLColor incol);

void syou(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[]);
void syou_downx(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[], int* num);

bool setti_puyo(TLColor ba[][kHeight], int aa, TLColor nx1, TLColor nx2, int setti_basyo[]);
int setti_puyo_1(TLColor ba[][kHeight], int eex, TLColor eecol);

int chousei_syoukyo(TLColor ba[][kHeight], int setti_basyo[]);
int chousei_syoukyo_2(TLColor ba[][kHeight], int setti_basyo[], int* chain, int dabuchk[], int* ichiren_kesi, int* score);
int chousei_syoukyo_3(TLColor bass[][kHeight], int[], int* poi2s, int* score, int tokus, int i2, int j2, int ruiseki_point);
int chousei_syoukyo_sc(TLColor ba[][kHeight], int setti_basyo[], int* score);
int hon_syoukyo(TLColor ba[][kHeight]);
int hon_syoukyo_score(TLColor ba[][kHeight], int* score, int* quick);

void setti_ojama(TLColor f[][kHeight], int numOjama);

}  // namespace test_lockit

#endif  // CPU_TEST_LOCKIT_FIELD_H_
