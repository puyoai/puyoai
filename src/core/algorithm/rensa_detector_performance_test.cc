#include "core/algorithm/rensa_detector.h"

#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "base/time_stamp_counter.h"
#include "core/field/core_field.h"

using namespace std;

TEST(RensaDetectorPerformanceTest, iteratePossibleRensas)
{
    TimeStampCounterData tsc;
    CoreField f(
        "  R G "
        "R GRBG"
        "RBGRBG"
        "RBGRBG");

    size_t size = 0;
    auto callback = [&](const CoreField&, const RensaResult&, const ColumnPuyoList&, const ColumnPuyoList&) {
        ++size;
    };

    for (int i = 0; i < 10000; ++i) {
        ScopedTimeStampCounter scts(&tsc);
        RensaDetector::iteratePossibleRensas(f, 1, callback);
    }

    cout << size << endl;
    tsc.showStatistics();
}
