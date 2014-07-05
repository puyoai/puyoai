#include "core/algorithm/rensa_detector.h"

#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "base/tsc.h"
#include "core/field/core_field.h"

using namespace std;

TEST(RensaDetectorPerformanceTest, iteratePossibleRensas)
{
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
        Tsc tsc("iteratePossibleRensas");
        RensaDetector::iteratePossibleRensas(f, 1, callback);
    }

    cout << size << endl;
    double average, variance;
    Tsc::GetStatistics("iteratePossibleRensas", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}
