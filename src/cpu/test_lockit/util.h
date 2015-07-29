#ifndef CPU_TEST_LOCKIT_UTIL_H_
#define CPU_TEST_LOCKIT_UTIL_H_

#include "core/puyo_color.h"

#include "color.h"
#include "lockit_constant.h"

class CoreField;

namespace test_lockit {

// TODO: Remove following functions.
// Field converter
CoreField toCoreField(PuyoColor[6][kHeight]);
void toTLField(const CoreField&, PuyoColor[6][kHeight]);

}  // namespace test_lockit

#endif  // CPU_TEST_LOCKIT_UTIL_H_
