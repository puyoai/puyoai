#include "core/algorithm/plan.h"

#include <iostream>
#include <gtest/gtest.h>

#include "base/tsc.h"
#include "core/field/core_field.h"
#include "core/kumipuyo.h"

using namespace std;

TEST(PlanPerformanceTest, Empty44)
{
    CoreField f;
    KumipuyoSeq seq("RRGGYYBB");

    // Since seq has 4 kumipuyo, this won't test all kumipuyo possibilities.
    for (int i = 0; i < 100; i++) {
        Tsc tsc("PlanEmpty44");
        Plan::iterateAvailablePlans(f, seq, 4, [](const RefPlan&){});
    }

    double average, variance;
    Tsc::GetStatistics("PlanEmpty44", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PlanPerformanceTest, Filled44)
{
    CoreField f("http://www.inosendo.com/puyo/rensim/??500000400000500000400000540000540000564560456456456456456456");
    KumipuyoSeq seq("RRGGYYBB");

    // Since seq has 4 kumipuyo, this won't test all kumipuyo possibilities.
    for (int i = 0; i < 100; i++) {
        Tsc tsc("PlanFilled44");
        Plan::iterateAvailablePlans(f, seq, 4, [](const RefPlan&){});
    }

    double average, variance;
    Tsc::GetStatistics("PlanFilled44", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PlanPerformanceTest, Empty23)
{
    CoreField f;
    KumipuyoSeq seq("RRGG");

    // Since seq has 2 kumipuyo, this will try all kumipuyo color possibilities.
    for (int i = 0; i < 10; i++) {
        Tsc tsc("PlanEmpty23");
        Plan::iterateAvailablePlans(f, seq, 3, [](const RefPlan&){});
    }

    double average, variance;
    Tsc::GetStatistics("PlanEmpty23", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PlanPerformanceTest, Filled23)
{
    CoreField f("http://www.inosendo.com/puyo/rensim/??500000400000500000400000540000540000564560456456456456456456");
    KumipuyoSeq seq("BBGG");

    // Since seq has 2 kumipuyo, this won't test all kumipuyo possibilities.
    for (int i = 0; i < 10; i++) {
        Tsc tsc("PlanFilled23");
        Plan::iterateAvailablePlans(f, seq, 3, [](const RefPlan&){});
    }

    double average, variance;
    Tsc::GetStatistics("PlanFilled23", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PlanPerformanceTest, Empty24)
{
    CoreField f;
    KumipuyoSeq seq("RRGG");

    // Since seq has 2 kumipuyo, this will try all kumipuyo color possibilities.
    for (int i = 0; i < 10; i++) {
        Tsc tsc("PlanEmpty24");
        Plan::iterateAvailablePlans(f, seq, 4, [](const RefPlan&){});
    }

    double average, variance;
    Tsc::GetStatistics("PlanEmpty24", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}

TEST(PlanPerformanceTest, Filled24)
{
    CoreField f("http://www.inosendo.com/puyo/rensim/??500000400000500000400000540000540000564560456456456456456456");
    KumipuyoSeq seq("BBGG");

    // Since seq has 4 kumipuyo, this won't test all kumipuyo possibilities.
    for (int i = 0; i < 10; i++) {
        Tsc tsc("PlanFilled24");
        Plan::iterateAvailablePlans(f, seq, 4, [](const RefPlan&){});
    }

    double average, variance;
    Tsc::GetStatistics("PlanFilled24", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}
