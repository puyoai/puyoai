#ifndef CPU_TEST_LOCKIT_FIELD_H_
#define CPU_TEST_LOCKIT_FIELD_H_

#include "lockit_constant.h"

namespace test_lockit {

bool IsTLFieldEmpty(const int field[6][kHeight]);

bool IsColorPuyo(const int c);

}  // namespace test_lockit

#endif  // CPU_TEST_LOCKIT_FIELD_H_

