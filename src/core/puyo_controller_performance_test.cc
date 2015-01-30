#include "core/puyo_controller.h"

#include <iostream>
#include <gtest/gtest.h>

#include "base/time_stamp_counter.h"
#include "core/core_field.h"
#include "core/decision.h"

using namespace std;

TEST(PuyoControllerPerformanceTest, empty)
{
    TimeStampCounterData tsc;

    CoreField f;

    for (int i = 0; i < 100; ++i) {
        Decision d(6, 3);
        ScopedTimeStampCounter stsc(&tsc);
        PuyoController::findKeyStroke(f, d);
    }

    tsc.showStatistics();
}

TEST(PuyoControllerPerformanceTest, unreachable)
{
    TimeStampCounterData tsc;

    CoreField f(
        " O O  "
        " O O  " // 12
        " O O  "
        " O O  "
        " O O  "
        " O O  " // 8
        " O O  "
        " O O  "
        " O O  "
        " O O  " // 4
        " O O  "
        " O O  "
        " O O  ");

    for (int i = 0; i < 100; ++i) {
        Decision d(6, 3);
        ScopedTimeStampCounter stsc(&tsc);
        PuyoController::findKeyStroke(f, d);
    }

    tsc.showStatistics();
}
