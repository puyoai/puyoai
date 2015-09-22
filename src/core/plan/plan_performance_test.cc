#include "core/plan/plan.h"

#include <gtest/gtest.h>

#include "base/time_stamp_counter.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq.h"

using namespace std;

TEST(PlanPerformanceTest, Empty44)
{
    TimeStampCounterData tsc;

    CoreField f;
    KumipuyoSeq seq("RRGGYYBB");

    // Since seq has 4 kumipuyo, this won't test all kumipuyo possibilities.
    for (int i = 0; i < 100; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        Plan::iterateAvailablePlans(f, seq, 4, [](const RefPlan&){});
    }

    tsc.showStatistics();
}

TEST(PlanPerformanceTest, Filled44)
{
    TimeStampCounterData tsc;
    CoreField f("B....."
                "R....."
                "B....."
                "R....."
                "BR...."
                "BR...."
                "BYRBY."
                "RBYRBY"
                "RBYRBY"
                "RBYRBY");
    KumipuyoSeq seq("RRGGYYBB");

    // Since seq has 4 kumipuyo, this won't test all kumipuyo possibilities.
    for (int i = 0; i < 100; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        Plan::iterateAvailablePlans(f, seq, 4, [](const RefPlan&){});
    }

    tsc.showStatistics();
}

TEST(PlanPerformanceTest, Empty23)
{
    TimeStampCounterData tsc;
    CoreField f;
    KumipuyoSeq seq("RRGG");

    // Since seq has 2 kumipuyo, this will try all kumipuyo color possibilities.
    for (int i = 0; i < 10; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        Plan::iterateAvailablePlans(f, seq, 3, [](const RefPlan&){});
    }

    tsc.showStatistics();
}

TEST(PlanPerformanceTest, Filled23)
{
    TimeStampCounterData tsc;
    CoreField f("B....."
                "R....."
                "B....."
                "R....."
                "BR...."
                "BR...."
                "BYRBY."
                "RBYRBY"
                "RBYRBY"
                "RBYRBY");
    KumipuyoSeq seq("BBGG");

    // Since seq has 2 kumipuyo, this won't test all kumipuyo possibilities.
    for (int i = 0; i < 10; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        Plan::iterateAvailablePlans(f, seq, 3, [](const RefPlan&){});
    }

    tsc.showStatistics();
}

TEST(PlanPerformanceTest, Empty24)
{
    TimeStampCounterData tsc;
    CoreField f;
    KumipuyoSeq seq("RRGG");

    // Since seq has 2 kumipuyo, this will try all kumipuyo color possibilities.
    for (int i = 0; i < 10; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        Plan::iterateAvailablePlans(f, seq, 4, [](const RefPlan&){});
    }

    tsc.showStatistics();
}

TEST(PlanPerformanceTest, Filled24)
{
    TimeStampCounterData tsc;
    CoreField f("B....."
                "R....."
                "B....."
                "R....."
                "BR...."
                "BR...."
                "BYRBY."
                "RBYRBY"
                "RBYRBY"
                "RBYRBY");
    KumipuyoSeq seq("BBGG");

    // Since seq has 4 kumipuyo, this won't test all kumipuyo possibilities.
    for (int i = 0; i < 10; i++) {
        ScopedTimeStampCounter stsc(&tsc);
        Plan::iterateAvailablePlans(f, seq, 4, [](const RefPlan&){});
    }

    tsc.showStatistics();
}
