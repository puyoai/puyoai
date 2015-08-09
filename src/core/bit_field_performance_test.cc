#include "core/bit_field.h"

#include <gtest/gtest.h>

#include "base/base.h"
#include "base/time_stamp_counter.h"

using namespace std;

TEST(BitFieldPerformanceTest, hash)
{
    const int N = 1000000;

    BitField original;
    size_t expected = original.hash();

    TimeStampCounterData tscd;

    for (int i = 0; i < N; ++i) {
        BitField bf;
        ScopedTimeStampCounter stsc(&tscd);
        EXPECT_EQ(expected, bf.hash());
    }

    tscd.showStatistics();
}
