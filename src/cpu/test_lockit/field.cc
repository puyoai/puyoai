#include "field.h"

#include "color.h"

namespace test_lockit {

bool IsTLFieldEmpty(const int field[6][kHeight])
{
    for (int i = 0; i < 6; ++i) {
        if (field[i][0] != TLColor::EMPTY) {
            return false;
        }
    }
    return true;
}

}  // namespace test_lockit
