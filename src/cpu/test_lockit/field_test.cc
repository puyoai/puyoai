#include "field.h"

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/puyo_color.h"
#include "lockit_constant.h"
#include "rensa_result.h"
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

    PuyoColor field[6][kHeight];
    toTLField(cf, field);

    int expect_points = 0;
    // We can re-use |point| array.
    Check point[6][12] {};
    for (const TestData& data : data_set) {
        int num = 0;
        saiki(field, point, data.x, data.y, &num, field[data.x][data.y]);

        EXPECT_EQ(data.expect_num, num);
        expect_points += data.expect_num;
    }

    int count_point = 0;
    for (int x = 0; x < 6; ++x) {
        for (int y = 0; y < 12; ++y) {
            if (point[x][y] != Check::Unchecked)
                ++count_point;
        }
    }
    EXPECT_EQ(expect_points, count_point);
}

TEST(FieldTest, countNormalColor13)
{
    CoreField cf(
        "G....." // 14
        "R....." // 13
        "Y....." // 12
        "B....."
        "G....."
        "R....."
        "Y....." // 8
        "B....."
        "G....."
        "R....."
        "Y....." // 4
        "B....."
        "G....."
        "R.OOO."
    );

    PuyoColor field[6][kHeight];
    toTLField(cf, field);

    EXPECT_EQ(13, countNormalColor13(field));
}

TEST(FieldTest, settiOjama)
{
    CoreField cf(
        "O....." // 13
        "OO...." // 12
        "OOO..."
        "OOOO.."
        "OOOOO."
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
    );

    PuyoColor field[6][kHeight];
    toTLField(cf, field);

    setti_ojama(field, 18);

    EXPECT_EQ(PuyoColor::OJAMA, field[0][12]);
    EXPECT_EQ(PuyoColor::OJAMA, field[1][12]);
    EXPECT_EQ(PuyoColor::OJAMA, field[2][12]);
    EXPECT_EQ(PuyoColor::OJAMA, field[3][12]);
    EXPECT_EQ(PuyoColor::EMPTY, field[4][12]);
    EXPECT_EQ(PuyoColor::EMPTY, field[5][12]);

    EXPECT_EQ(PuyoColor::EMPTY, field[0][13]);
    EXPECT_EQ(PuyoColor::EMPTY, field[1][13]);
    EXPECT_EQ(PuyoColor::EMPTY, field[2][13]);
    EXPECT_EQ(PuyoColor::EMPTY, field[3][13]);
    EXPECT_EQ(PuyoColor::EMPTY, field[4][13]);
    EXPECT_EQ(PuyoColor::EMPTY, field[5][13]);
}

TEST(FieldTest, simulate_basic)
{
    CoreField cf("RRRR..");
    PuyoColor field[6][kHeight];
    toTLField(cf, field);

    TLRensaResult result = simulate(field);
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(40, result.score);
    EXPECT_EQ(4, result.num_vanished);
    EXPECT_TRUE(result.quick);
    EXPECT_EQ(1, result.num_connections[0]);
}

TEST(FieldTest, simulate_chain)
{
    CoreField cf(
        "..B..."
        "..BBYB"
        "RRRRBB");
    PuyoColor field[6][kHeight];
    toTLField(cf, field);

    TLRensaResult result = simulate(field);
    EXPECT_EQ(2, result.chains);
    EXPECT_EQ(700, result.score);
    EXPECT_EQ(10, result.num_vanished);
    EXPECT_FALSE(result.quick);
    EXPECT_EQ(1, result.num_connections[0]);
    EXPECT_EQ(1, result.num_connections[1]);
}

TEST(FieldTest, simulate_1_double)
{
    CoreField cf(
        "..BBBB"
        "RRRRRR");
    PuyoColor field[6][kHeight];
    toTLField(cf, field);

    TLRensaResult result = simulate(field);
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(600, result.score);
    EXPECT_EQ(10, result.num_vanished);
    EXPECT_TRUE(result.quick);
    EXPECT_EQ(2, result.num_connections[0]);
}

TEST(FieldTest, simulate_double_cornercase)
{
    CoreField cf(
        "BB..BB"
        "BB..BB");
    PuyoColor field[6][kHeight];
    toTLField(cf, field);

    TLRensaResult result = simulate(field);
    EXPECT_EQ(1, result.chains);
    EXPECT_EQ(80, result.score);
    EXPECT_EQ(8, result.num_vanished);
    EXPECT_TRUE(result.quick);
    EXPECT_EQ(2, result.num_connections[0]);
}

TEST(FieldTest, simulate_2_double)
{
    CoreField cf(
        ".R...."
        ".R...."
        ".RYR.R"
        "RYYYRR");
    PuyoColor field[6][kHeight];
    toTLField(cf, field);

    TLRensaResult result = simulate(field);
    EXPECT_EQ(2, result.chains);
    EXPECT_EQ(680, result.score);
    EXPECT_EQ(12, result.num_vanished);
    EXPECT_TRUE(result.quick);
    EXPECT_EQ(1, result.num_connections[0]);
    EXPECT_EQ(2, result.num_connections[1]);
}

}  // namespace test_lockit
