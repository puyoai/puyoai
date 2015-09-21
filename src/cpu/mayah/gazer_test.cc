#include "gazer.h"

#include <memory>
#include <gtest/gtest.h>

#include "core/kumipuyo_seq.h"
#include "core/probability/puyo_set_probability.h"

using namespace std;

class GazerTest : public testing::Test {
protected:
    virtual void SetUp() override
    {
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

    GazeResult gazeResult = gazer_->gazeResult();
    // Gazer should find a rensa with the first BY.
    // 2280 = basic score of 4 rensa.
    EXPECT_EQ(2280, gazeResult.estimateMaxScore(100, PlayerState())) << gazeResult.toRensaInfoString();
}

TEST_F(GazerTest, estimateMaxScoreUsingRensaIsOngoing)
{
    CoreField f;
    KumipuyoSeq seq("BYRRGG");
    gazer_->gaze(100, f, seq);

    PlayerState enemy;
    enemy.currentChain = 1;
    enemy.currentChainStartedFrameId = 100;
    enemy.currentRensaResult = RensaResult(10, 36840, 300, false);

    GazeResult gazeResult = gazer_->gazeResult();

    // Since ongoing rensa will finish in frameId=400, we can estimate rensa score is 36840 until frameId=400.
    EXPECT_EQ(36840, gazeResult.estimateMaxScore(200, enemy)) << gazeResult.toRensaInfoString();
    EXPECT_EQ(36840, gazeResult.estimateMaxScore(300, enemy)) << gazeResult.toRensaInfoString();
    EXPECT_EQ(36840, gazeResult.estimateMaxScore(400, enemy)) << gazeResult.toRensaInfoString();
}
