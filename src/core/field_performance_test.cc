#include "core/core_field.h"

#include <gtest/gtest.h>

#include "base/base.h"
#include "base/time_stamp_counter.h"
#include "core/rensa_result.h"

using namespace std;

TEST(FieldPerformanceTest, Copy)
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

TEST(FieldPerformanceTest, Simulate_Empty)
{
    TimeStampCounterData tsc;

    for (int i = 0; i < 1000000; i++) {
        CoreField f;
        ScopedTimeStampCounter tsct(&tsc);
        f.simulate();
    }

    tsc.showStatistics();
}

TEST(FieldPerformanceTest, Simulate_Filled)
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

TEST(FieldPerformanceTest, Simulate_Filled_Track)
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

TEST(FieldPerformanceTest, countConnectedPuyosEmpty)
{
    TimeStampCounterData tsc;

    for (int i = 0; i < 1000000; i++) {
        CoreField f;

        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(f.countConnectedPuyos(3, 12), 72);
    }

    tsc.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyosFilled)
{
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

    TimeStampCounterData tsc;
    for (int i = 0; i < 1000000; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(3, f.countConnectedPuyos(3, 2));
    }

    tsc.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyosMax4)
{
    CoreField f("OOOGGG"
                "OOOGGG" // 12
                "OOOOOO"
                "ORRRRO"
                "ORRRRO"
                "OOOOOO" // 8
                "BBBOBB"
                "GGGOGG"
                "RRRORR"
                "OOOOOO" // 4
                "RGBORO"
                "RGBOGO"
                "RGBOBO");

    TimeStampCounterData tsc;
    for (int i = 0; i < 1000000; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(1, f.countConnectedPuyosMax4(5, 1));
        EXPECT_EQ(2, f.countConnectedPuyosMax4(5, 5));
        EXPECT_EQ(3, f.countConnectedPuyosMax4(1, 5));
        EXPECT_EQ(3, f.countConnectedPuyosMax4(6, 12));
        EXPECT_LE(4, f.countConnectedPuyosMax4(2, 9));
    }

    tsc.showStatistics();
}
