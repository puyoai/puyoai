#include "pattern.h"

#include "core/core_field.h"
#include "core/puyo_color.h"

#include <sstream>

#include <gtest/gtest.h>

namespace peria {

class PatternForTest : public Pattern {
 public:
  using Pattern::AppendField;
  using Pattern::GetScore;
  using Pattern::ParseBook;
  using Pattern::num_puyos_;
  using Pattern::pattern_;
  using Pattern::score_;
};

class PatternTest : public testing::Test {};

TEST_F(PatternTest, ReadBook) {
  const std::string book_str("NAME: Begin1\n" "SCORE: 1000\n" "MAX: 8\n"
                             ".AA...\n" ".BB...\n");
  std::istringstream iss(book_str);

  PatternForTest pattern;
  pattern.ParseBook(iss);

  EXPECT_EQ(1000, pattern.score_);
}

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
