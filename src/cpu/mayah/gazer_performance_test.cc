#include "gazer.h"

#include <iostream>

#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include "base/time_stamp_counter.h"
#include "core/plan/plan.h"
#include "core/core_field.h"
#include "core/frame_request.h"
#include "core/kumipuyo_seq.h"
#include "core/probability/puyo_set_probability.h"

#include "mayah_ai.h"

using namespace std;

static void runTest(const CoreField& cf, const KumipuyoSeq& seq)
{
    Gazer gazer;
    gazer.initialize(1);
    gazer.gaze(100, cf, seq);
}

TEST(GazerPerformanceTest, pattern1)
{
    CoreField f(
        "    RB"
        " B GGG"
        "GG YBR"
        "YG YGR"
        "GBYBGR"
        "BBYYBG"
        "GYBGRG"
        "GGYGGR"
        "YYBBBR");
    KumipuyoSeq seq("RBRGRYYG");

    runTest(f, seq);
}

TEST(GazerPerformanceTest, pattern2)
{
    CoreField cf(
        "B....."
        "YY...."
        "BRY..."
        "BRY..."
        "BGRB.."
        "RBGRBG"
        "RBGRBG"
        "RBGRBG");
    KumipuyoSeq seq("RBRGRYYG");

    runTest(cf, seq);
}

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    google::ParseCommandLineFlags(&argc, &argv, true);

    return RUN_ALL_TESTS();
}
