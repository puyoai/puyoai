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

TEST_F(GazerTest, estimateMaxScoreUsingRensaIsOngoing)
{
    CoreField f;
    KumipuyoSeq seq("BYRRGG");
    gazer_->gaze(100, f, seq);
    gazer_->setOngoingRensa(OngoingRensaInfo(RensaResult(10, 36840, 300, false), 400));

    // Since ongoing rensa will finish in frameId=400, we can estimate rensa score is 36840 until frameId=400.
    EXPECT_EQ(36840, gazer_->estimateMaxScore(200)) << gazer_->toRensaInfoString();
    EXPECT_EQ(36840, gazer_->estimateMaxScore(300)) << gazer_->toRensaInfoString();
    EXPECT_EQ(36840, gazer_->estimateMaxScore(400)) << gazer_->toRensaInfoString();
}
