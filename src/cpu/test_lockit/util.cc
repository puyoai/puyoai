#include "cpu/test_lockit/util.h"

#include "core/core_field.h"
#include "core/puyo_color.h"

namespace test_lockit {

CoreField toCoreField(PuyoColor f[6][kHeight]) {
    CoreField cf;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < kHeight; ++j) {
            if (f[i][j] == PuyoColor::EMPTY)
                break;
            cf.dropPuyoOn(i + 1, f[i][j]);
        }
    }
    return cf;
}

void toTLField(const CoreField& cf, PuyoColor f[6][kHeight]) {
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < kHeight; ++j) {
            f[i][j] = cf.color(i + 1, j + 1);
        }
    }
}

}  // namespace test_lockit
