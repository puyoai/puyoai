#include "core/core_field.h"

#include <gtest/gtest.h>

#include "base/base.h"
#include "base/time_stamp_counter.h"
#include "core/field_bits.h"
#include "core/rensa_result.h"

using namespace std;

TEST(FieldPerformanceTest, copy)
{
    TimeStampCounterData tsc;

    CoreField f(".G.BRG"
                "GBRRYR"
                "RRYYBY"
                "RGYRBR"
                "YGYRBY"
                "YGBGYR"
                "GRBGYR"
                "BRBYBY"
                "RYYBYY"
                "BRBYBR"
                "BGBYRR"
                "YGBGBG"
                "RBGBGG");

    for (int i = 0; i < 1000000; i++) {
        ScopedTimeStampCounter tsct(&tsc);
        CoreField f2(f);
        UNUSED_VARIABLE(f2);
    }

    tsc.showStatistics();
}

TEST(FieldPerformanceTest, simulateEmpty)
{
    TimeStampCounterData tsc;

    for (int i = 0; i < 1000000; i++) {
        CoreField f;
        ScopedTimeStampCounter tsct(&tsc);
        f.simulate();
    }

    tsc.showStatistics();
}

TEST(FieldPerformanceTest, simulateFilled)
{
    TimeStampCounterData tsc;

    for (int i = 0; i < 100000; i++) {
        CoreField f(".G.BRG"
                    "GBRRYR"
                    "RRYYBY"
                    "RGYRBR"
                    "YGYRBY"
                    "YGBGYR"
                    "GRBGYR"
                    "BRBYBY"
                    "RYYBYY"
                    "BRBYBR"
                    "BGBYRR"
                    "YGBGBG"
                    "RBGBGG");
        ScopedTimeStampCounter tsct(&tsc);
        f.simulate();
    }

    tsc.showStatistics();
}

TEST(FieldPerformanceTest, simulateFilledTracking)
{
    TimeStampCounterData tsc;

    for (int i = 0; i < 100000; i++) {
        CoreField f(".G.BRG"
                    "GBRRYR"
                    "RRYYBY"
                    "RGYRBR"
                    "YGYRBY"
                    "YGBGYR"
                    "GRBGYR"
                    "BRBYBY"
                    "RYYBYY"
                    "BRBYBR"
                    "BGBYRR"
                    "YGBGBG"
                    "RBGBGG");


        ScopedTimeStampCounter stsc(&tsc);
        RensaChainTracker tracker;
        f.simulate(&tracker);
    }

    tsc.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyos_empty)
{
    const PlainField f;

    TimeStampCounterData tsc;
    TimeStampCounterData tscMax4;
    TimeStampCounterData tscBits;
    TimeStampCounterData tscPreBits;

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(72, f.countConnectedPuyos(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscMax4);
        EXPECT_LE(4, f.countConnectedPuyosMax4(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscBits);
        FieldBits fb(f, PuyoColor::RED);
        EXPECT_EQ(72, fb.expand(3, 1).popcount());
    }

    FieldBits fb(f, PuyoColor::RED);
    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscPreBits);
        EXPECT_EQ(72, fb.expand(3, 1).popcount());
    }

    tsc.showStatistics();
    tscMax4.showStatistics();
    tscBits.showStatistics();
    tscPreBits.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyos_1)
{
    const PlainField f("..R...");

    TimeStampCounterData tsc;
    TimeStampCounterData tscMax4;
    TimeStampCounterData tscBits;
    TimeStampCounterData tscPreBits;

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(1, f.countConnectedPuyos(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscMax4);
        EXPECT_EQ(1, f.countConnectedPuyosMax4(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscBits);
        FieldBits fb(f, PuyoColor::RED);
        EXPECT_EQ(1, fb.expand(3, 1).popcount());
    }

    FieldBits fb(f, PuyoColor::RED);
    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscPreBits);
        EXPECT_EQ(1, fb.expand(3, 1).popcount());
    }

    tsc.showStatistics();
    tscMax4.showStatistics();
    tscBits.showStatistics();
    tscPreBits.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyos_2)
{
    const PlainField f("..RR..");

    TimeStampCounterData tsc;
    TimeStampCounterData tscMax4;
    TimeStampCounterData tscBits;
    TimeStampCounterData tscPreBits;

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(2, f.countConnectedPuyos(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscMax4);
        EXPECT_EQ(2, f.countConnectedPuyosMax4(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscBits);
        FieldBits fb(f, PuyoColor::RED);
        EXPECT_EQ(2, fb.expand(3, 1).popcount());
    }

    FieldBits fb(f, PuyoColor::RED);
    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscPreBits);
        EXPECT_EQ(2, fb.expand(3, 1).popcount());
    }

    tsc.showStatistics();
    tscMax4.showStatistics();
    tscBits.showStatistics();
    tscPreBits.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyos_3)
{
    const PlainField f("..RRR.");

    TimeStampCounterData tsc;
    TimeStampCounterData tscMax4;
    TimeStampCounterData tscBits;
    TimeStampCounterData tscPreBits;

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(3, f.countConnectedPuyos(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscMax4);
        EXPECT_EQ(3, f.countConnectedPuyosMax4(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscBits);
        FieldBits fb(f, PuyoColor::RED);
        EXPECT_EQ(3, fb.expand(3, 1).popcount());
    }

    FieldBits fb(f, PuyoColor::RED);
    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscPreBits);
        EXPECT_EQ(3, fb.expand(3, 1).popcount());
    }

    tsc.showStatistics();
    tscMax4.showStatistics();
    tscBits.showStatistics();
    tscPreBits.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyos_4)
{
    const PlainField f("..RRRR");

    TimeStampCounterData tsc;
    TimeStampCounterData tscMax4;
    TimeStampCounterData tscBits;
    TimeStampCounterData tscPreBits;

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(4, f.countConnectedPuyos(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscMax4);
        EXPECT_EQ(4, f.countConnectedPuyosMax4(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscBits);
        FieldBits fb(f, PuyoColor::RED);
        EXPECT_EQ(4, fb.expand(3, 1).popcount());
    }

    FieldBits fb(f, PuyoColor::RED);
    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscPreBits);
        EXPECT_EQ(4, fb.expand(3, 1).popcount());
    }

    tsc.showStatistics();
    tscMax4.showStatistics();
    tscBits.showStatistics();
    tscPreBits.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyos_4_2)
{
    const PlainField f(
        "..R..."
        "..R..."
        "..R..."
        "..R...");

    TimeStampCounterData tsc;
    TimeStampCounterData tscMax4;
    TimeStampCounterData tscBits;
    TimeStampCounterData tscPreBits;

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(4, f.countConnectedPuyos(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscMax4);
        EXPECT_EQ(4, f.countConnectedPuyosMax4(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscBits);
        FieldBits fb(f, PuyoColor::RED);
        EXPECT_EQ(4, fb.expand(3, 1).popcount());
    }

    FieldBits fb(f, PuyoColor::RED);
    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscPreBits);
        EXPECT_EQ(4, fb.expand(3, 1).popcount());
    }

    tsc.showStatistics();
    tscMax4.showStatistics();
    tscBits.showStatistics();
    tscPreBits.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyos_4_3)
{
    const PlainField f(
        "..RR.."
        "..RR..");

    TimeStampCounterData tsc;
    TimeStampCounterData tscMax4;
    TimeStampCounterData tscBits;
    TimeStampCounterData tscPreBits;

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(4, f.countConnectedPuyos(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscMax4);
        EXPECT_EQ(4, f.countConnectedPuyosMax4(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscBits);
        FieldBits fb(f, PuyoColor::RED);
        EXPECT_EQ(4, fb.expand(3, 1).popcount());
    }

    FieldBits fb(f, PuyoColor::RED);
    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscPreBits);
        EXPECT_EQ(4, fb.expand(3, 1).popcount());
    }

    tsc.showStatistics();
    tscMax4.showStatistics();
    tscBits.showStatistics();
    tscPreBits.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyos_evil)
{
    const PlainField f(
        "RRRRRR" // 12
        "R....R"
        "R..R.R"
        "R.RR.R"
        "R.R..R" // 8
        "R.RR.R"
        "R..R.R"
        "R.RR.R"
        "R.R..R" // 4
        "R.RRRR"
        "R....."
        "RRRRRR");

    TimeStampCounterData tsc;
    TimeStampCounterData tscMax4;
    TimeStampCounterData tscBits;
    TimeStampCounterData tscPreBits;

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(4, f.countConnectedPuyos(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscMax4);
        EXPECT_EQ(4, f.countConnectedPuyosMax4(3, 1));
    }

    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscBits);
        FieldBits fb(f, PuyoColor::RED);
        EXPECT_EQ(4, fb.expand(3, 1).popcount());
    }

    FieldBits fb(f, PuyoColor::RED);
    for (int i = 0; i < 10000000; i++) {
        ScopedTimeStampCounter stsc(&tscPreBits);
        EXPECT_EQ(4, fb.expand(3, 1).popcount());
    }

    tsc.showStatistics();
    tscMax4.showStatistics();
    tscBits.showStatistics();
    tscPreBits.showStatistics();
}
