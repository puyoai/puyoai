#include "duel/field_realtime.h"

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "core/constant.h"
#include "core/kumipuyo.h"
#include "core/state.h"
#include "duel/field_realtime.h"
#include "duel/frame_context.h"

using namespace std;

class FieldRealtimeTest : public ::testing::Test {
protected:
    virtual void SetUp()
    {
        string sequence = "RRGGBBYY";
        f_.reset(new FieldRealtime(0, sequence));
    }

    virtual void TearDown()
    {
        f_.reset();
    }

    unique_ptr<FieldRealtime> f_;
};

TEST_F(FieldRealtimeTest, Move1)
{
    CoreField cf(
        "      " // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    f_->forceSetField(cf);
    f_->skipLevelSelect();

    while (true) {
        FrameContext context;
        Key key = f_->getKey(Decision(5, 1));
        if (!f_->playOneFrame(KeySet(key), &context))
            break;
    }

    CoreField expected(
        "    RR" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    EXPECT_EQ(expected, f_->field());
}

TEST_F(FieldRealtimeTest, Move2)
{
    CoreField cf(
        "      " // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    f_->forceSetField(cf);
    f_->skipLevelSelect();

    while (true) {
        FrameContext context;
        Key key = f_->getKey(Decision(6, 2));
        if (!f_->playOneFrame(KeySet(key), &context))
            break;
    }

    CoreField expected(
        "     R"
        "     R" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    EXPECT_EQ(expected, f_->field());
}

TEST_F(FieldRealtimeTest, Move3)
{
    CoreField cf(
        "      " // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    f_->forceSetField(cf);
    f_->skipLevelSelect();

    while (true) {
        FrameContext context;
        Key key = f_->getKey(Decision(1, 2));
        if (!f_->playOneFrame(KeySet(key), &context))
            break;
    }

    CoreField expected(
        "R     "
        "R     " // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    EXPECT_EQ(expected, f_->field());
}

TEST_F(FieldRealtimeTest, Move4)
{
    CoreField cf(
        "    OO" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    f_->forceSetField(cf);
    f_->skipLevelSelect();

    while (true) {
        FrameContext context;
        Key key = f_->getKey(Decision(6, 3));
        if (!f_->playOneFrame(KeySet(key), &context))
            break;
    }

    CoreField expected(
        "    RR" // 13
        "    OO" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    EXPECT_EQ(expected, f_->field());
}
