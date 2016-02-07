#include "core/core_field.h"

#include <gtest/gtest.h>

#include "base/base.h"
#include "base/time_stamp_counter.h"
#include "core/bit_field.h"
#include "core/field_bits.h"
#include "core/rensa_result.h"

using namespace std;

static void runCountConnectedPuyosTest(const PlainField& f, int expected, int x, int y)
{
    const int N = 1000000;

    TimeStampCounterData none;
    TimeStampCounterData tscPlainField;
    TimeStampCounterData tscPlainFieldMax4;
    TimeStampCounterData tscBitField;
    TimeStampCounterData tscBitFieldMax4;
    TimeStampCounterData tscBitFieldWithColor;
    TimeStampCounterData tscBitFieldMax4WithColor;

    const int expected4 = expected >= 4 ? 4 : expected;

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&none);
    }

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscPlainField);
        EXPECT_EQ(expected, f.countConnectedPuyos(x, y));
    }

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscPlainFieldMax4);
        EXPECT_LE(expected4, f.countConnectedPuyosMax4(x, y));
    }

    BitField bf(f);

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscBitField);
        EXPECT_EQ(expected, bf.countConnectedPuyos(x, y));
    }

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscBitFieldMax4);
        EXPECT_LE(expected4, bf.countConnectedPuyosMax4(x, y));
    }

    PuyoColor c = bf.color(x, y);

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscBitFieldWithColor);
        EXPECT_EQ(expected, bf.countConnectedPuyos(x, y, c));
    }

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&tscBitFieldMax4WithColor);
        EXPECT_LE(expected4, bf.countConnectedPuyosMax4(x, y, c));
    }

    cout << "overhead: " << endl;
    none.showStatistics();
    cout << "PlainField::countConnectedPuyos: " << endl;
    tscPlainField.showStatistics();
    cout << "PlainField::countConnectedPuyosMax4: " << endl;
    tscPlainFieldMax4.showStatistics();
    cout << "FieldBits::countConnectedField: " << endl;
    tscBitField.showStatistics();
    cout << "FieldBits::countConnectedFieldMax4: " << endl;
    tscBitFieldMax4.showStatistics();
    cout << "FieldBits::countConnectedField (with color): " << endl;
    tscBitFieldWithColor.showStatistics();
    cout << "FieldBits::countConnectedFieldMax4 (with color): " << endl;
    tscBitFieldMax4WithColor.showStatistics();
}

static void runSimulation(const CoreField& original)
{
    const int expectedChain = CoreField(original).simulate().chains;
    const int N = 100000;

    TimeStampCounterData none;
    TimeStampCounterData tscCoreField;
    TimeStampCounterData tscBitField;
    TimeStampCounterData tscBitFieldFast;

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&none);
    }

    for (int i = 0; i < N; i++) {
        CoreField cf(original);
        ScopedTimeStampCounter stsc(&tscCoreField);
        EXPECT_EQ(expectedChain, cf.simulate().chains);
    }

    for (int i = 0; i < N; i++) {
        BitField bf(original.bitField());
        ScopedTimeStampCounter stsc(&tscBitField);
        EXPECT_EQ(expectedChain, bf.simulate().chains);
    }

    for (int i = 0; i < N; i++) {
        BitField bf(original.bitField());
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tscBitFieldFast);
        EXPECT_EQ(expectedChain, bf.simulateFast(&tracker));
    }

#if defined(__AVX2__) and defined(__BMI2__)
    TimeStampCounterData tscBitFieldAVX2;
    TimeStampCounterData tscBitFieldFastAVX2;

    for (int i = 0; i < N; ++i) {
        BitField bf(original.bitField());
        BitField::SimulationContext context;
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tscBitFieldAVX2);
        EXPECT_EQ(expectedChain, bf.simulateAVX2(&context, &tracker).chains);
    }

    for (int i = 0; i < N; ++i) {
        BitField bf(original.bitField());
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tscBitFieldFastAVX2);
        EXPECT_EQ(expectedChain, bf.simulateFastAVX2(&tracker));
    }
#endif // __AVX2__ and __BMI2__

    cout << "overhead: " << endl;
    none.showStatistics();
    cout << "CoreField: " << endl;
    tscCoreField.showStatistics();
    cout << "BitField: " << endl;
    tscBitField.showStatistics();
    cout << "BitField (fast): " << endl;
    tscBitFieldFast.showStatistics();

#if defined(__AVX2__) and defined(__BMI2__)
    cout << "BitField AVX2: " << endl;
    tscBitFieldAVX2.showStatistics();
    cout << "BitField (fast) AVX2: " << endl;
    tscBitFieldFastAVX2.showStatistics();
#endif
}

static void runVanishDrop(const CoreField& original)
{
    const int expectedChain = CoreField(original).simulate().chains;
    const int N = 100000;

    TimeStampCounterData none;
    TimeStampCounterData tscCoreField;
    TimeStampCounterData tscBitField;
    TimeStampCounterData tscBitFieldFast;

    for (int i = 0; i < N; i++) {
        ScopedTimeStampCounter stsc(&none);
    }

    for (int i = 0; i < N; i++) {
        CoreField cf(original);
        CoreField::SimulationContext context;
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tscCoreField);
        while (cf.vanishDrop(&context, &tracker).score > 0) {
            // do nothing.
        }
        EXPECT_EQ(expectedChain, context.currentChain - 1);
    }

    for (int i = 0; i < N; i++) {
        BitField bf(original.bitField());
        BitField::SimulationContext context;
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tscBitField);
        while (bf.vanishDrop(&context, &tracker).score > 0) {
            // do nothing.
        }
        EXPECT_EQ(expectedChain, context.currentChain - 1);
    }

    for (int i = 0; i < N; i++) {
        BitField bf(original.bitField());
        BitField::SimulationContext context;
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tscBitFieldFast);
        while (bf.vanishDropFast(&context, &tracker)) {
            // do nothing.
        }
        EXPECT_EQ(expectedChain, context.currentChain - 1);
    }

#if defined(__AVX2__) and defined(__BMI2__)
    TimeStampCounterData tscBitFieldAVX2;
    TimeStampCounterData tscBitFieldFastAVX2;

    for (int i = 0; i < N; i++) {
        BitField bf(original.bitField());
        BitField::SimulationContext context;
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tscBitFieldAVX2);
        while (bf.vanishDropAVX2(&context, &tracker).score > 0) {
            // do nothing.
        }
        EXPECT_EQ(expectedChain, context.currentChain - 1);
    }

    for (int i = 0; i < N; i++) {
        BitField bf(original.bitField());
        BitField::SimulationContext context;
        RensaNonTracker tracker;
        ScopedTimeStampCounter stsc(&tscBitFieldFastAVX2);
        while (bf.vanishDropFastAVX2(&context, &tracker)) {
            // do nothing.
        }
        EXPECT_EQ(expectedChain, context.currentChain - 1);
    }
#endif // __AVX2__ and __BMI2__

    cout << "overhead: " << endl;
    none.showStatistics();
    cout << "CoreField: " << endl;
    tscCoreField.showStatistics();
    cout << "BitField: " << endl;
    tscBitField.showStatistics();
    cout << "BitField (fast): " << endl;
    tscBitFieldFast.showStatistics();

#if defined(__AVX2__) and defined(__BMI2__)
    cout << "BitField AVX2: " << endl;
    tscBitFieldAVX2.showStatistics();
    cout << "BitField (fast) AVX2: " << endl;
    tscBitFieldFastAVX2.showStatistics();
#endif
}

TEST(FieldPerformanceTest, copy)
{
    TimeStampCounterData tsc;

    CoreField f(".G.BRG"
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

    for (int i = 0; i < 1000000; i++) {
        ScopedTimeStampCounter tsct(&tsc);
        CoreField f2(f);
        UNUSED_VARIABLE(f2);
    }

    tsc.showStatistics();
}

TEST(FieldPerformanceTest, simulate_empty)
{
    CoreField cf;
    runSimulation(cf);
}

TEST(FieldPerformanceTest, simulate_easy)
{
    CoreField cf(".RBRB."
                 "RBRBR."
                 "RBRBR."
                 "RBRBRR");
    runSimulation(cf);
}

TEST(FieldPerformanceTest, simulate_evil)
{
    CoreField cf("..BB.."
                 "..GGB."
                 ".GYYG."
                 ".BBBYB"
                 "RRRRBY");
    runSimulation(cf);
}

TEST(FieldPerformanceTest, simulate_filled)
{
    CoreField cf(".G.BRG"
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
    runSimulation(cf);
}

TEST(FieldPerformanceTest, vanishDrop_filled)
{
    CoreField cf(".G.BRG"
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
    runVanishDrop(cf);
}

TEST(FieldPerformanceTest, countConnectedPuyos_empty)
{
    const PlainField f;
    runCountConnectedPuyosTest(f, 72, 3, 12);
}

TEST(FieldPerformanceTest, countConnectedPuyos_1)
{
    const PlainField f("..R...");
    runCountConnectedPuyosTest(f, 1, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_2)
{
    const PlainField f("..RR..");
    runCountConnectedPuyosTest(f, 2, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_3)
{
    const PlainField f("..RRR.");
    runCountConnectedPuyosTest(f, 3, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_4)
{
    const PlainField f("..RRRR");
    runCountConnectedPuyosTest(f, 4, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_4_2)
{
    const PlainField f(
        "..R..."
        "..R..."
        "..R..."
        "..R...");
    runCountConnectedPuyosTest(f, 4, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_4_3)
{
    const PlainField f(
        "..RR.."
        "..RR..");
    runCountConnectedPuyosTest(f, 4, 3, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_6)
{
    const PlainField f(
        "RRRRRR");
    runCountConnectedPuyosTest(f, 6, 1, 1);
}

TEST(FieldPerformanceTest, countConnectedPuyos_evil)
{
    const PlainField f(
        "RRRRRR" // 12
        "R....R"
        "R..R.R"
        "R.RR.R"
        "R.R..R" // 8
        "R.RR.R"
        "R..R.R"
        "R.RR.R"
        "R.R..R" // 4
        "R.RRRR"
        "R....."
        "RRRRRR");

    runCountConnectedPuyosTest(f, 44, 6, 1);
}
