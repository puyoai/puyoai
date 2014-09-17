#include "gazer.h"

#include <memory>
#include <gtest/gtest.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/kumipuyo.h"

using namespace std;

class GazerTest : public testing::Test {
protected:
    virtual void SetUp() override
    {
        TsumoPossibility::initialize();
        gazer_.reset(new Gazer());
        gazer_->initialize(100);
    }

    virtual void TearDown() override
    {
        gazer_.reset();
    }

    std::unique_ptr<Gazer> gazer_;
};

TEST_F(GazerTest, dontCrash)
{
    // Should not crash in this test case.

    CoreField f(" O    "
                " O O  " // 12
                "OO OOO"
                "OOOOOO"
                "OGOOOO"
                "OYOOOO" // 8
                "OOOOOO"
                "OOOOOO"
                "OOOOOO"
                "OOOOOO" // 4
                "OOOOOO"
                "OBOYOO"
                "BBOBBR");

    KumipuyoSeq seq("BBRBYB");
    gazer_->gaze(100, f, seq);
}

TEST_F(GazerTest, feasibleRensas)
{
    CoreField f(
        "BRBG  "
        "BBRBBB"
        "RRYGGG");
    KumipuyoSeq seq("BYRRGG");
    gazer_->gaze(100, f, seq);

    // Gazer should find a rensa with the first BY.
    // 2280 = basic score of 4 rensa.
    EXPECT_EQ(2280, gazer_->estimateMaxScore(100)) << gazer_->toRensaInfoString();
}

TEST_F(GazerTest, empty)
{
    CoreField f;
    KumipuyoSeq seq("BYRRGG");
    gazer_->gaze(100, f, seq);

    for (int i = 100; i < 1000; i += 10)
        cout << i << ' ' << gazer_->estimateMaxScore(i) << endl;

    // Gazer should find a rensa with the first BY.
    // 2280 = basic score of 4 rensa.
    EXPECT_EQ(2280, gazer_->estimateMaxScore(100)) << gazer_->toRensaInfoString();
}
