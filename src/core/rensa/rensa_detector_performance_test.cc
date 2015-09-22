#include "core/rensa/rensa_detector.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <iostream>

#include "base/time_stamp_counter.h"
#include "core/core_field.h"

class ColumnPuyoList;
class RensaChainTrackResult;
struct RensaResult;

using namespace std;

TEST(RensaDetectorPerformanceTest, detectIteratively_Drop)
{
    TimeStampCounterData tsc;

    const CoreField original(
        "  R G "
        "R GRBG"
        "RBGRBG"
        "RBGRBG");

    auto callback = [&](CoreField&& cf, const ColumnPuyoList&) -> RensaResult {
        return cf.simulate();
    };

    for (int i = 0; i < 10000; ++i) {
        ScopedTimeStampCounter stsc(&tsc);
        RensaDetector::detectIteratively(original, RensaDetectorStrategy::defaultDropStrategy(), 3, callback);
    }

    tsc.showStatistics();
}

TEST(RensaDetectorPerformanceTest, detectIteratively_Float)
{
    TimeStampCounterData tsc;

    const CoreField original(
        "  R G "
        "R GRBG"
        "RBGRBG"
        "RBGRBG");

    auto callback = [&](CoreField&& cf, const ColumnPuyoList&) -> RensaResult {
        return cf.simulate();
    };

    for (int i = 0; i < 10000; ++i) {
        ScopedTimeStampCounter stsc(&tsc);
        RensaDetector::detectIteratively(original, RensaDetectorStrategy::defaultFloatStrategy(), 3, callback);
    }

    tsc.showStatistics();
}

TEST(RensaDetectorPerformanceTest, detectIteratively_Extend)
{
    TimeStampCounterData tsc;

    const CoreField original(
        "  R G "
        "R GRBG"
        "RBGRBG"
        "RBGRBG");

    auto callback = [&](CoreField&& cf, const ColumnPuyoList&) -> RensaResult {
        return cf.simulate();
    };

    for (int i = 0; i < 10000; ++i) {
        ScopedTimeStampCounter stsc(&tsc);
        RensaDetector::detectIteratively(original, RensaDetectorStrategy::defaultExtendStrategy(), 3, callback);
    }

    tsc.showStatistics();
}
