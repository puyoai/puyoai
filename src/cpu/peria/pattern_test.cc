#include "pattern.h"

#include "core/core_field.h"
#include "core/puyo_color.h"

#include <gtest/gtest.h>

namespace peria {

class PatternForTest : public Pattern {
 public:
  using Pattern::AppendField;
  using Pattern::GetScore;
  using Pattern::num_puyos_;
  using Pattern::pattern_;
  using Pattern::score_;
};

class PatternTest : public testing::Test {};

TEST_F(PatternTest, GetScore) {
  PatternForTest pattern;
  pattern.score_ = 1000;
  pattern.num_puyos_ = 4;
  Pattern::MatchingCounts counts;
  counts['A'][PuyoColor::RED] = 2;
  counts['B'][PuyoColor::BLUE] = 2;

  EXPECT_EQ(1000, pattern.GetScore(counts));
}

TEST_F(PatternTest, Matching) {
  PatternForTest pattern;
  pattern.AppendField(".AA...");
  pattern.AppendField(".BB...");
  pattern.score_ = 1000;
  pattern.num_puyos_ = 4;

  CoreField field("0RR0000BB000");
  EXPECT_EQ(1000, pattern.Match(field));
}

}  // namespace peria
