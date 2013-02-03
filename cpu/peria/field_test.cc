#include "field.h"

#include <algorithm>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "plan.h"

using namespace std;

namespace {

void TestUrl(string url, int expected_chains, int expected_score) {
  Field f(url);
  int chains = 1, score = 0, frames = 0;
  f.Simulate(&chains, &score, &frames);
  EXPECT_EQ(expected_chains, chains);
  EXPECT_EQ(expected_score, score);
}

#define ARRAYSIZE(A) (sizeof(A) / sizeof(*A))

}  // namespace

class FieldTest : public ::testing::Test {
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(FieldTest, ChainAndScoreTest) {
  const string kUrlPrefix = "http://www.inosendo.com/puyo/rensim/??";
  TestUrl(kUrlPrefix + "50745574464446676456474656476657564547564747676466766"
          "747674757644657575475755", 19, 175080);
  TestUrl(kUrlPrefix + "50046776767574445475465744776766764467454545576747764"
          "4457474656455446775455646", 19, 175080);
  TestUrl(kUrlPrefix + "55005045504545104574507474507464506767406767405656705"
          "6567515167444416555155", 2, 38540);
  TestUrl(kUrlPrefix + "50550040455075451075745064745064645067674057674747574"
          "776567675156644415555155", 3, 43260);
  TestUrl(kUrlPrefix + "55004045507545177574546474546464546767445767414757477"
          "6567675156644415555155", 4, 50140);
  TestUrl(kUrlPrefix + "74555057645566645117574556474556474556767415767474757"
          "4776566615156644415555155", 5, 68700);
  TestUrl(kUrlPrefix + "44441111414141411411411141414441111441411111441441111"
          "4441114111141444141111141", 4, 4840);
  TestUrl(kUrlPrefix + "54554454445445454545454545454554545444554455445545454"
          "5545454554544445455455445", 9, 49950);
  TestUrl(kUrlPrefix + "44444654461144616456444154616656561545455144144411111"
          "1111111111111111111111111", 9, 32760);
  TestUrl(kUrlPrefix + "66754746655566167747147545144746166666154745747755644"
          "6776555744646476466744555", 18, 155980);
  TestUrl(kUrlPrefix + "44404414441411414441141141414414141441414144141411441"
          "1441144414141141414144144", 11, 47080);
  TestUrl(kUrlPrefix + "44444444444444444444444444444444444444444444444444444"
          "4444444444444444444", 1, 7200);
}

TEST_F(FieldTest, FramesTest) {
  int chains, score, frames;
  {
    // 1 Rensa, no drop.
    Field f("444400");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_AFTER_NO_DROP, frames);
  }
  {
    Field f("500000"
            "444400");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP,
              frames);
  }
  {
    Field f("500000"
            "400000"
            "444000");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP,
              frames);
  }
  {
    Field f("500000"
            "450000"
            "444000");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP,
              frames);
  }
  {
    Field f("500000"
            "455000"
            "444500");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 +
              FRAMES_AFTER_DROP + FRAMES_AFTER_VANISH + FRAMES_AFTER_NO_DROP,
              frames);
  }
  {
    Field f("560000"
            "455000"
            "444500");
    f.Simulate(&chains, &score, &frames);
    EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 +
              FRAMES_AFTER_DROP + FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE +
              FRAMES_AFTER_DROP, frames);
  }
}

TEST_F(FieldTest, FindAvailableControlsTest) {
  const string base = string(Field::WIDTH * (Field::HEIGHT - 1), '1');
  {
    Field f;
    vector<Decision> decisions;
    f.FindAvailableControls(false, &decisions);
    EXPECT_EQ(22, decisions.size());
  }
  {
    Field f;
    vector<Decision> decisions;
    f.FindAvailableControls(true, &decisions);
    EXPECT_EQ(11, decisions.size()) << f.GetDebugOutput();
  }
  {
    Field f(string("100010""110011") + base);
    vector<Decision> decisions;
    f.FindAvailableControls(false, &decisions);
    EXPECT_EQ(9, decisions.size()) << f.GetDebugOutput();
  }
  {
    Field f(string("100010""110011") + base);
    vector<Decision> decisions;
    f.FindAvailableControls(true, &decisions);
    EXPECT_EQ(5, decisions.size()) << f.GetDebugOutput();
  }
  {
    Field f(string(Field::WIDTH * Field::HEIGHT, '1'));
    vector<Decision> decisions;
    f.FindAvailableControls(true, &decisions);
    EXPECT_EQ(0, decisions.size()) << f.GetDebugOutput();
  }
}

TEST_F(FieldTest, FindAvailablePlansTest) {
  struct TestData {
    const char *tsumo;
    const int choice[4];
    const int max_score;
  } testdata[] = {
    {"444444", {1, 11, 11*11, 11*11*11}, 60 * 3},
    {"445566", {1, 11, 11*11, 11*11*11}, 0},
    {"456745", {1, 22, 22*22, 22*22*22}, 0},
    {"445675", {1, 11, 11*22, 11*22*22}, 0},
    {"456477", {1, 22, 22*22, 22*22*11}, 0},
    {"445644", {1, 11, 11*22, 11*22*11}, 40},
  };

  for (size_t i = 0; i < ARRAYSIZE(testdata); ++i) {
    Field f;
    f.SetColorSequence(testdata[i].tsumo);

    vector<Plan> plans[4];
    f.FindAvailablePlans(plans);
    for (int j = 0; j <= 3; ++j)
      EXPECT_EQ(testdata[i].choice[j], plans[j].size())
        << "(i, j) = (" << i << ", " << j << ")";

    int score = 0;
    for (int j = 0; j < testdata[i].choice[3]; ++j) {
      score = max(score, plans[3][j].score);
      //LOG(INFO) << plans[3][j].field.GetDebugOutput();
    }
    EXPECT_EQ(testdata[i].max_score, score);
  }

  // Death check
  {
    Field f(string(Field::WIDTH * Field::HEIGHT, '1'));
    f.SetColorSequence("444444");

    vector<Plan> plans[4];
    f.FindAvailablePlans(plans);
    EXPECT_EQ(1, plans[0].size());
    EXPECT_EQ(0, plans[1].size());
    EXPECT_EQ(0, plans[2].size());
    EXPECT_EQ(0, plans[3].size());
  }
}

int main(int argc, char **argv) {
  printf("Running main() from gtest_main.cc\n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
