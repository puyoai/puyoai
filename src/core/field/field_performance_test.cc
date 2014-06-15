#include "core/field/core_field.h"

#include <gtest/gtest.h>
#include <vector>

#include "base/tsc.h"
#include "core/field/rensa_result.h"

using namespace std;

TEST(FieldPerformanceTest, Copy)
{
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
        Tsc tsc("Copy");
        CoreField f2(f);
    }
    double average, variance;
    Tsc::GetStatistics("Copy", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(FieldPerformanceTest, Simulate_Empty)
{
    for (int i = 0; i < 1000000; i++) {
        CoreField f;
        Tsc tsc("Simulate_Empty");
        f.simulate();
    }

    double average, variance;
    Tsc::GetStatistics("Simulate_Empty", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(FieldPerformanceTest, Simulate_Filled)
{
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
        Tsc tsc("Simulate_Filled");
        f.simulate();
    }
    double average, variance;
    Tsc::GetStatistics("Simulate_Filled", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(FieldPerformanceTest, Simulate_Filled_Track)
{
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

        Tsc tsc("Simulate_Filled_Track");

        RensaTrackResult trackResult;
        f.simulateAndTrack(&trackResult);
    }

    double average, variance;
    Tsc::GetStatistics("Simulate_Filled_Track", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}
