#include "field.h"

#include <algorithm>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "base.h"

using namespace std;

class FieldTest : public ::testing::Test {
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(FieldTest, Construct_Url) {
  struct {
    const char* url;  // input
    int x, y;         // input
    char color;       // expected output
  } data[] = {
    {"444444", 1, 1, kRed},
    {"444444", 6, 1, kRed},
    {"444444", 1, 2, kEmpty},
    {"444444", 0, 1, kWall},
    {"444444", 1, 0, kWall},
    {"500000444444", 1, 2, kBlue},
    {"5444444", 6, 2, kBlue}, // 2nd : |     B|, bottom : |RRRRRR|
  };
  for (int i = 0; i < ARRAYSIZE(data); ++i) {
    Field field(data[i].url);
    EXPECT_EQ(data[i].color, field.Get(data[i].x, data[i].y));
    LOG_IF(INFO, data[i].color != field.Get(data[i].x, data[i].y))
        << field.GetDebugOutput();
  }
}

TEST_F(FieldTest, Vanishable) {
  struct {
    const char* field;  // input
    int x, y;           // input
    bool able;          // expected output
  } data[] = {
    {"444000", 1, 1, false},
    {"444400", 1, 1, true},
    {"4444", 1, 1, false},  // (1,1) is empty
    {"440000440000", 1, 1, true},
  };
  for (int i = 0; i < ARRAYSIZE(data); ++i) {
    Field field(data[i].field);
    EXPECT_EQ(data[i].able, field.Vanishable(data[i].x, data[i].y));
    LOG_IF(INFO, data[i].able != field.Vanishable(data[i].x, data[i].y))
        << "for i = " << i;

  }
}

TEST_F(FieldTest, Simulate) {
  struct {
    const char* field;  // input
    int chains;         // input
    int ex_chains;      // expected output
    int ex_score;       // expected output
  } data[] = {
    {"444", 1, 0, 0},
    {"4444", 1, 1, 40},
  };
  for (int i = 0; i < ARRAYSIZE(data); ++i) {
    Field field(data[i].field);
    int chains = data[i].chains;
    int score = 0;
    int frames = 0;
    field.Simulate(&chains, &score, &frames);
    EXPECT_EQ(data[i].ex_score, score);
    EXPECT_EQ(data[i].ex_chains, chains);
  }
}
