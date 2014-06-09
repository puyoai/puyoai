#include <gtest/gtest.h>
#include <vector>

#include "field.h"
#include "../../util/tsc.h"

using namespace std;

TEST(PerformanceTest, Copy) {
    LF f("http://www.inosendo.com/puyo/rensim/??50745574464446676456474656476657564547564747676466766747674757644657575475755");
  for (int i = 0; i < 10000000; i++) {
    Tsc tsc("Copy");
    LF f2(f);
  }
  double average, variance;
  Tsc::GetStatistics("Copy", &average, &variance);
  cout << "average: " << average << endl;
  cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, Simulate_Empty) {
  for (int i = 0; i < 1000000; i++) {
    Tsc tsc("Simulate_Empty");
    LF f;
    int chains, score, frames;
    f.Simulate(&chains, &score, &frames);
  }
  double average, variance;
  Tsc::GetStatistics("Simulate_Empty", &average, &variance);
  cout << "average: " << average << endl;
  cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, Simulate_Filled) {
  for (int i = 0; i < 100000; i++) {
    Tsc tsc("Simulate_Filled");
    LF f("http://www.inosendo.com/puyo/rensim/??50745574464446676456474656476657564547564747676466766747674757644657575475755");
    int chains, score, frames;
    f.Simulate(&chains, &score, &frames);
  }
  double average, variance;
  Tsc::GetStatistics("Simulate_Filled", &average, &variance);
  cout << "average: " << average << endl;
  cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindAvailablePlans_Empty) {
  for (int i = 0; i < 500; i++) {
    Tsc tsc("FindAvailablePlans_Empty");
    LF f;
    const string& next = LF::parseNext("456745");
    vector<LP> plans;
    f.FindAvailablePlans(next, &plans);
  }
  double average, variance;
  Tsc::GetStatistics("FindAvailablePlans_Empty", &average, &variance);
  cout << "average: " << average << endl;
  cout << "variance: " << variance << endl;
}

TEST(PerformanceTest, FindAvailablePlans_Filled) {
  for (int i = 0; i < 500; i++) {
    Tsc tsc("FindAvailablePlans_Filled");
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
  double average, variance;
  Tsc::GetStatistics("FindAvailablePlans_Filled", &average, &variance);
  cout << "average: " << average << endl;
  cout << "variance: " << variance << endl;
}
