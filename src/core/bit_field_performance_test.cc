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

TEST(BitFieldPerformanceTest, bitfield_simulate_filled)
{
    const int N = 1000000;

    TimeStampCounterData tsc;
    BitField bfOriginal(
        ".G.BRG"
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

    for (int i = 0; i < N; i++) {
        BitField bf(bfOriginal);
        BitField::SimulationContext context;
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(19, bf.simulate(&context, &tracker).chains);
    }

    tsc.showStatistics();
}

TEST(BitFieldPerformanceTest, bitfield_simulate_fast_filled)
{
    const int N = 1000000;

    TimeStampCounterData tsc;
    BitField bfOriginal(
        ".G.BRG"
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

    for (int i = 0; i < N; i++) {
        BitField bf(bfOriginal);
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(19, bf.simulateFast(&tracker));
    }

    tsc.showStatistics();
}

#if defined(__AVX2__) && defined(__BMI2__)
TEST(BitFieldPerformanceTest, bitfield_simulate_avx2_filled)
{
    const int N = 1000000;

    TimeStampCounterData tsc;
    BitField bfOriginal(
        ".G.BRG"
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

    for (int i = 0; i < N; i++) {
        BitField bf(bfOriginal);
        BitField::SimulationContext context;
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(19, bf.simulateAVX2(&context, &tracker).chains);
    }

    tsc.showStatistics();
}

TEST(BitFieldPerformanceTest, bitfield_simulate_fast_avx2_filled)
{
    const int N = 1000000;

    TimeStampCounterData tsc;
    BitField bfOriginal(
        ".G.BRG"
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

    for (int i = 0; i < N; i++) {
        BitField bf(bfOriginal);
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tsc);
        EXPECT_EQ(19, bf.simulateFastAVX2(&tracker));
    }

    tsc.showStatistics();
}
#endif // defined(__AVX2__) && defined(__BMI2__)
