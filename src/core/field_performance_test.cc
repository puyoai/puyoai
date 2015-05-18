#include "core/core_field.h"

#include <gtest/gtest.h>

#include "base/base.h"
#include "base/time_stamp_counter.h"
#include "core/field_bits.h"
#include "core/rensa_result.h"

using namespace std;

static void runCountConnectedPuyosTest(const PlainField& f, int expected, int x, int y)
{
    const int N = 1000000;

    TimeStampCounterData none;
    TimeStampCounterData tsc;
    TimeStampCounterData tscMax4;
    TimeStampCounterData tscBits;
    TimeStampCounterData tscBitsMax4;
    TimeStampCounterData tscPreBits;
    TimeStampCounterData tscPreBitsMax4;

    const int expected4 = expected >= 4 ? 4 : expected;

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&none);
    }

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(expected, f.countConnectedPuyos(x, y));
    }

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscMax4);
        EXPECT_LE(expected4, f.countConnectedPuyosMax4(x, y));
    }

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscBits);
        FieldBits fb(f, f.color(x, y));
        EXPECT_EQ(expected, FieldBits(x, y).expand(fb.masked()).popcount());
    }

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscBitsMax4);
        FieldBits fb(f, f.color(x, y));
        EXPECT_LE(expected4, FieldBits(x, y).expand4(fb.masked()).popcount());
    }

    FieldBits fb(f, f.color(x, y));
    fb = fb.masked();
    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscPreBits);
        EXPECT_EQ(expected, FieldBits(x, y).expand(fb).popcount());
    }

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscPreBitsMax4);
        EXPECT_LE(expected4, FieldBits(x, y).expand4(fb).popcount());
    }

    cout << "overhead: " << endl;
    none.showStatistics();
    cout << "normal: " << endl;
    tsc.showStatistics();
    cout << "max4: " << endl;
    tscMax4.showStatistics();
    cout << "bits normal: " << endl;
    tscBits.showStatistics();
    cout << "bits max4: " << endl;
    tscBitsMax4.showStatistics();
    cout << "prebits normal: " << endl;
    tscPreBits.showStatistics();
    cout << "prebits max4: " << endl;
    tscPreBitsMax4.showStatistics();
}

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
    runCountConnectedPuyosTest(f, 72, 3, 12);
}

TEST(FieldPerformanceTest, countConnectedPuyos_1)
{
    const PlainField f("..R...");
    runCountConnectedPuyosTest(f, 1, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_2)
{
    const PlainField f("..RR..");
    runCountConnectedPuyosTest(f, 2, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_3)
{
    const PlainField f("..RRR.");
    runCountConnectedPuyosTest(f, 3, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_4)
{
    const PlainField f("..RRRR");
    runCountConnectedPuyosTest(f, 4, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_4_2)
{
    const PlainField f(
        "..R..."
        "..R..."
        "..R..."
        "..R...");
    runCountConnectedPuyosTest(f, 4, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_4_3)
{
    const PlainField f(
        "..RR.."
        "..RR..");
    runCountConnectedPuyosTest(f, 4, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_6)
{
    const PlainField f(
        "RRRRRR");
    runCountConnectedPuyosTest(f, 6, 1, 1);
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

    runCountConnectedPuyosTest(f, 44, 6, 1);
}
