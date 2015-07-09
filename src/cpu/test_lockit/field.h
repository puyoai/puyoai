#ifndef CPU_TEST_LOCKIT_FIELD_H_
#define CPU_TEST_LOCKIT_FIELD_H_

#include "lockit_constant.h"

namespace test_lockit {

bool IsTLFieldEmpty(const int field[6][kHeight]);


void saiki(const int field[][kHeight], int point[][12], int x, int y, int* num, int incol);
void saiki_right(const int[][kHeight], int[][12], int, int, int*, int);
void saiki_left(const int[][kHeight], int[][12], int, int, int*, int);
void saiki_up(const int[][kHeight], int[][12], int, int, int*, int);
void saiki_down(const int[][kHeight], int[][12], int, int, int*, int);

}  // namespace test_lockit

#endif  // CPU_TEST_LOCKIT_FIELD_H_
