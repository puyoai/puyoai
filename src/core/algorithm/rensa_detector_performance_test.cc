#include "core/algorithm/rensa_detector.h"

#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "base/tsc.h"
#include "core/field/core_field.h"

using namespace std;

TEST(RensaDetectorPerformanceTest, findPossibleRensas)
{
    CoreField f(
        "  R G "
        "R GRBG"
        "RBGRBG"
        "RBGRBG");

    size_t size = 0;
    for (int i = 0; i < 100; ++i) {
        Tsc tsc("findPossibleRensas");
        vector<PossibleRensaInfo> pris = RensaDetector::findPossibleRensas(f, 1);
        size = pris.size();
    }

    cout << size << endl;
    double average, variance;
    Tsc::GetStatistics("findPossibleRensas", &average, &variance);
    cout << "average: " << average << endl;
    cout << "variance: " << variance << endl;
}
