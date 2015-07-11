#include "field.h"

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "lockit_constant.h"
#include "util.h"

namespace test_lockit {

TEST(FieldTest, Saiki) {
    CoreField cf(
        "BBGGYY"    // 12 (invisible)
        "BBGGYY"    // 11
        "@@@@@@"
        "@@@@@@"
        "@@@@@@"
        "@@@@@@"    // 7
        "@@@@@@"
        "GGRRRR"
        "YRRGGR"
        "RRBBBR"    // 3
        "@RRRRR"
        "@@@@@@"
        "RBYG@@");  // 0 (y-index in TLField)
    struct TestData {
        int x;
        int y;
        int expect_num;
    } data_set[] = {
        {0, 0, 1}, {1, 0, 1}, {2, 0, 1}, {3, 0, 1},
        {1, 3, 15}, {2, 3, 3},
        {1, 11, 2}, {2, 11, 2}
    };

    // We can re-use |point| array.
    int point[6][12] {};
    for (const TestData& data : data_set) {
        int field[6][kHeight];

        int num = 0;
        toTLField(cf, field);
        saiki(field, point, data.x, data.y, &num, field[data.x][data.y]);

        EXPECT_EQ(data.expect_num, num);
    }

    int count_point = 0;
    for (int x = 0; x < 6; ++x) {
        for (int y = 0; y < 12; ++y) {
            if (point[x][y])
                ++count_point;
        }
    }
    EXPECT_EQ(26, count_point);
}

}  // namespace test_lockit
