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
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    for (int i = 0; i < 100; ++i) {
        Decision d(6, 3);
        ScopedTimeStampCounter stsc(&tsc);
        PuyoController::findKeyStrokeByDijkstra(f, mks, d);
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

    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    for (int i = 0; i < 100; ++i) {
        Decision d(6, 3);
        ScopedTimeStampCounter stsc(&tsc);
        PuyoController::findKeyStrokeByDijkstra(f, mks, d);
    }

    tsc.showStatistics();
}
