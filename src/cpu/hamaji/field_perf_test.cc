#include <gtest/gtest.h>
#include <vector>

#include "base/base.h"
#include "base/time_stamp_counter.h"
#include "field.h"

using namespace std;

TEST(PerformanceTest, FindAvailablePlans_Empty) {
  TimeStampCounterData tsc;

  for (int i = 0; i < 500; i++) {
    ScopedTimeStampCounter stsc(&tsc);
    LF f;
    const string& next = LF::parseNext("456745");
    vector<LP> plans;
    f.FindAvailablePlans(next, &plans);
  }
  tsc.showStatistics();
}

TEST(PerformanceTest, FindAvailablePlans_Filled) {
  TimeStampCounterData tsc;

  for (int i = 0; i < 500; i++) {
    ScopedTimeStampCounter stsc(&tsc);
    LF f("446676"
         "456474"
         "656476"
         "657564"
         "547564"
         "747676"
         "466766"
         "747674"
         "757644"
         "657575"
         "475755");
    const string& next = LF::parseNext("456745");
    vector<LP> plans;
    f.FindAvailablePlans(next, &plans);
  }
  tsc.showStatistics();
}
