#include "core/algorithm/rensa_detector.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <iostream>

#include "base/time_stamp_counter.h"
#include "core/core_field.h"

class ColumnPuyoList;
class RensaChainTrackResult;
struct RensaResult;

using namespace std;

TEST(RensaDetectorPerformanceTest, iteratePossibleRensas)
{
    TimeStampCounterData tsc1;
    TimeStampCounterData tsc2;

    CoreField f(
        "  R G "
        "R GRBG"
        "RBGRBG"
        "RBGRBG");

    size_t size1 = 0;
    size_t size2 = 0;
    auto callback1 = [&](const CoreField&, const RensaResult&, const ColumnPuyoList&, const ColumnPuyoList&) {
        ++size1;
    };
    auto callback2 = [&](const CoreField&, const RensaResult&, const ColumnPuyoList&, const ColumnPuyoList&, const RensaChainTrackResult&) {
        ++size2;
    };

    for (int i = 0; i < 10000; ++i) {
        ScopedTimeStampCounter scts(&tsc1);
        RensaDetector::iteratePossibleRensas(f, 1, RensaDetectorStrategy::defaultDropStrategy(), callback1);
    }

    for (int i = 0; i < 10000; ++i) {
        ScopedTimeStampCounter scts(&tsc2);
        RensaDetector::iteratePossibleRensasWithTracking(f, 1, RensaDetectorStrategy::defaultDropStrategy(), callback2);
    }

    cout << size1 << endl;
    cout << size2 << endl;
    tsc1.showStatistics();
    cout << endl;
    tsc2.showStatistics();
}
