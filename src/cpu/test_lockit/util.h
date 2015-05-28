#ifndef CPU_TEST_LOCKIT_UTIL_H_
#define CPU_TEST_LOCKIT_UTIL_H_

#include "color.h"
#include "core/puyo_color.h"

class CoreField;

// Color coverter
TLColor toTLColor(PuyoColor pc);
PuyoColor toPuyoColor(TLColor c);

// TODO: Remove following functions.
// Field converter
CoreField toCoreField(int[6][18]);
void toTLField(const CoreField&, int[6][18]);

#endif  // CPU_TEST_LOCKIT_UTIL_H_
