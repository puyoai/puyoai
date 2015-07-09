#ifndef CPU_TEST_LOCKIT_FIELD_H_
#define CPU_TEST_LOCKIT_FIELD_H_

#include "lockit_constant.h"

namespace test_lockit {

bool IsTLFieldEmpty(const int field[6][kHeight]);

void saiki(const int field[][kHeight], int point[][12], int x, int y, int* num, int incol);

}  // namespace test_lockit

#endif  // CPU_TEST_LOCKIT_FIELD_H_
