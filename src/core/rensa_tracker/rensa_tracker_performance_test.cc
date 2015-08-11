#include "core/rensa_tracker/rensa_tracker.h"

#include <iostream>

#include <gtest/gtest.h>

#include "base/base.h"
#include "base/time_stamp_counter.h"
#include "core/core_field.h"

using namespace std;

namespace {

static void runSimulation(const CoreField& original)
{
    const int expectedChain = CoreField(original).simulate().chains;

    const int N = 100000;

    TimeStampCounterData tscCoreField;
    TimeStampCounterData tscCoreFieldWithRensaChainTracker;

    for (int i = 0; i < N; i++) {
        CoreField cf(original);
        ScopedTimeStampCounter stsc(&tscCoreField);
        EXPECT_EQ(expectedChain, cf.simulate().chains);
    }

    for (int i = 0; i < N; ++i) {
        CoreField cf(original);
        RensaChainTracker tracker;
        ScopedTimeStampCounter stsc(&tscCoreFieldWithRensaChainTracker);
        EXPECT_EQ(expectedChain, cf.simulate(&tracker).chains);
    }

    cout << "CoreField: " << endl;
    tscCoreField.showStatistics();
    cout << "CoreField (RensaChainTracker): " << endl;
    tscCoreFieldWithRensaChainTracker.showStatistics();
}

} // namespace anonymous

TEST(RensaTrackerPerformanceTest, filled)
{
    CoreField original(".G.BRG"
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

    runSimulation(original);
}
