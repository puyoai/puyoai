#ifndef CPU_TEST_LOCKIT_UTIL_H_
#define CPU_TEST_LOCKIT_UTIL_H_

#include "core/puyo_color.h"

#include "color.h"

class CoreField;

namespace test_lockit {

// TODO: Remove following functions.
// Field converter
CoreField toCoreField(int[6][18]);
void toTLField(const CoreField&, int[6][18]);

}  // namespace test_lockit

#endif  // CPU_TEST_LOCKIT_UTIL_H_
