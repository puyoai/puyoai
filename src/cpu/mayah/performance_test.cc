#include <gtest/gtest.h>
#include <vector>

#include "base/tsc.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_detector.h"
#include "core/algorithm/rensa_info.h"
#include "field.h"

using namespace std;

TEST(PerformanceTest, Copy)
{
    Field f("http://www.inosendo.com/puyo/rensim/??50745574464446676456474656476657564547564747676466766747674757644657575475755");
    for (int i = 0; i < 1000000; i++) {
        Tsc tsc("Copy");
        Field f2(f);
    }
    double average, variance;
    Tsc::GetStatistics("Copy", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, Simulate_Empty)
{
    for (int i = 0; i < 1000000; i++) {
        Field f;
        Tsc tsc("Simulate_Empty");
        f.simulate();
    }
    double average, variance;
    Tsc::GetStatistics("Simulate_Empty", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, Simulate_Filled)
{
    for (int i = 0; i < 100000; i++) {
        Field f("http://www.inosendo.com/puyo/rensim/??50745574464446676456474656476657564547564747676466766747674757644657575475755");
        Tsc tsc("Simulate_Filled");
        f.simulate();
    }
    double average, variance;
    Tsc::GetStatistics("Simulate_Filled", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, Simulate_Filled_Track)
{
    for (int i = 0; i < 100000; i++) {
        Field f("http://www.inosendo.com/puyo/rensim/??50745574464446676456474656476657564547564747676466766747674757644657575475755");
        Tsc tsc("Simulate_Filled_Track");

        RensaTrackResult trackResult;
        f.simulateAndTrack(&trackResult);
    }
    double average, variance;
    Tsc::GetStatistics("Simulate_Filled_Track", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindPossibleRensas0)
{
    for (int i = 0; i < 100000; i++) {
        Field f("http://www.inosendo.com/puyo/rensim/??400000456700567400456740456740");
        Tsc tsc("FindPossibleRensas0");
        vector<PossibleRensaInfo> result = RensaDetector::findPossibleRensas(f, 0);
    }

    double average, variance;
    Tsc::GetStatistics("FindPossibleRensas0", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindPossibleRensas1)
{
    for (int i = 0; i < 10000; i++) {
        Field f("http://www.inosendo.com/puyo/rensim/??400000456700567400456740456740");
        Tsc tsc("FindPossibleRensas1");
        vector<PossibleRensaInfo> result = RensaDetector::findPossibleRensas(f, 1);
    }

    double average, variance;
    Tsc::GetStatistics("FindPossibleRensas1", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindPossibleRensas2)
{
    for (int i = 0; i < 1000; i++) {
        Field f("http://www.inosendo.com/puyo/rensim/??400000456700567400456740456740");
        Tsc tsc("FindPossibleRensas2");
        vector<PossibleRensaInfo> result = RensaDetector::findPossibleRensas(f, 2);
    }

    double average, variance;
    Tsc::GetStatistics("FindPossibleRensas2", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindPossibleRensas3)
{
    for (int i = 0; i < 100; i++) {
        Field f("http://www.inosendo.com/puyo/rensim/??400000456700567400456740456740");
        Tsc tsc("FindPossibleRensas3");
        vector<PossibleRensaInfo> result = RensaDetector::findPossibleRensas(f, 3);
    }

    double average, variance;
    Tsc::GetStatistics("FindPossibleRensas3", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindPossibleRensas4)
{
    for (int i = 0; i < 10; i++) {
        Field f("http://www.inosendo.com/puyo/rensim/??400000456700567400456740456740");
        Tsc tsc("FindPossibleRensas4");
        vector<PossibleRensaInfo> result = RensaDetector::findPossibleRensas(f, 4);
    }

    double average, variance;
    Tsc::GetStatistics("FindPossibleRensas4", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}
