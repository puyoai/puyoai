#ifndef CPU_TEST_LOCKIT_FIELD_H_
#define CPU_TEST_LOCKIT_FIELD_H_

#include "lockit_constant.h"

namespace test_lockit {

bool IsTLFieldEmpty(const int field[6][kHeight]);

void saiki(const int field[][kHeight], int point[][12], int x, int y, int* num, int incol);
void saiki_3(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol);
void saiki_4(int ba[][kHeight], int x, int y, int* num, int incol);

void syou(int ba[][kHeight], int x, int y, int incol, int flg[]);
void syou_downx(int ba[][kHeight], int x, int y, int incol, int flg[], int* num);

bool setti_puyo(int ba[][kHeight], int aa, int nx1, int nx2, int setti_basyo[]);
int setti_puyo_1(int ba[][kHeight], int eex, int eecol);

}  // namespace test_lockit

#endif  // CPU_TEST_LOCKIT_FIELD_H_
