#include "core/core_field.h"

#include <gtest/gtest.h>
#include <vector>

#include "base/time_stamp_counter.h"
#include "core/rensa_result.h"

using namespace std;

TEST(FieldPerformanceTest, Copy)
{
    TimeStampCounterData tsc;

    CoreField f("050745"
                "574464"
                "446676"
                "456474"
                "656476"
                "657564"
                "547564"
                "747676"
                "466766"
                "747674"
                "757644"
                "657575"
                "475755");

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
        CoreField f("050745"
                    "574464"
                    "446676"
                    "456474"
                    "656476"
                    "657564"
                    "547564"
                    "747676"
                    "466766"
                    "747674"
                    "757644"
                    "657575"
                    "475755");
        ScopedTimeStampCounter tsct(&tsc);
        f.simulate();
    }

    tsc.showStatistics();
}

TEST(FieldPerformanceTest, Simulate_Filled_Track)
{
    TimeStampCounterData tsc;

    for (int i = 0; i < 100000; i++) {
        CoreField f("050745"
                    "574464"
                    "446676"
                    "456474"
                    "656476"
                    "657564"
                    "547564"
                    "747676"
                    "466766"
                    "747674"
                    "757644"
                    "657575"
                    "475755");

        ScopedTimeStampCounter stsc(&tsc);
        RensaTrackResult trackResult;
        f.simulateAndTrack(&trackResult);
    }

    tsc.showStatistics();
}

TEST(FieldPerformanceTest, countConnectedPuyos)
{
    TimeStampCounterData tsc;

    for (int i = 0; i < 1000000; i++) {
        CoreField f;

        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(f.countConnectedPuyos(3, 12), 72);
    }

    tsc.showStatistics();
}
