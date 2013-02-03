#include <gtest/gtest.h>
#include <vector>

#include "field.h"
#include "plan.h"
#include "rensainfo.h"
#include "../../util/tsc.h"

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
        Tsc tsc("Simulate_Empty");
        Field f;
        BasicRensaInfo rensaInfo;
        f.simulate(rensaInfo);
    }
    double average, variance;
    Tsc::GetStatistics("Simulate_Empty", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, Simulate_Filled)
{
    for (int i = 0; i < 100000; i++) {
        Tsc tsc("Simulate_Filled");
        Field f("http://www.inosendo.com/puyo/rensim/??50745574464446676456474656476657564547564747676466766747674757644657575475755");
        BasicRensaInfo rensaInfo;
        f.simulate(rensaInfo);
    }
    double average, variance;
    Tsc::GetStatistics("Simulate_Filled", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindPossibleRensas1)
{
    for (int i = 0; i < 10000; i++) {
        Tsc tsc("FindPossibleRensas1");
        Field f("http://www.inosendo.com/puyo/rensim/??400000456700567400456740456740");
        vector<PossibleRensaInfo> result;
        f.findPossibleRensas(result, 1);
    }

    double average, variance;
    Tsc::GetStatistics("FindPossibleRensas1", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindPossibleRensas2)
{
    for (int i = 0; i < 1000; i++) {
        Tsc tsc("FindPossibleRensas2");
        Field f("http://www.inosendo.com/puyo/rensim/??400000456700567400456740456740");
        vector<PossibleRensaInfo> result;
        f.findPossibleRensas(result, 2);
    }

    double average, variance;
    Tsc::GetStatistics("FindPossibleRensas2", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindPossibleRensas3)
{
    for (int i = 0; i < 100; i++) {
        Tsc tsc("FindPossibleRensas3");
        Field f("http://www.inosendo.com/puyo/rensim/??400000456700567400456740456740");
        vector<PossibleRensaInfo> result;
        f.findPossibleRensas(result, 3);
    }

    double average, variance;
    Tsc::GetStatistics("FindPossibleRensas3", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindPossibleRensas4)
{
    for (int i = 0; i < 100; i++) {
        Tsc tsc("FindPossibleRensas4");
        Field f("http://www.inosendo.com/puyo/rensim/??400000456700567400456740456740");
        vector<PossibleRensaInfo> result;
        f.findPossibleRensas(result, 4);
    }

    double average, variance;
    Tsc::GetStatistics("FindPossibleRensas4", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}
