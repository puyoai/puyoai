#include "pattern.h"

#include "core/core_field.h"
#include "core/field_constant.h"
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
  using Pattern::max_puyos_;
  using Pattern::pattern_;
  using Pattern::score_;
  using Pattern::name_;
};

class PatternTest : public testing::Test {
 public:
  virtual void SetUp() {
    std::istringstream iss("");
    Pattern::ReadBook(iss);
  }

  virtual void TearDown() {
    std::istringstream iss("");
    Pattern::ReadBook(iss);
  }
};

TEST_F(PatternTest, ParseBook) {
  const std::string book_str("NAME: Test\n" "SCORE: 1000\n" "MAX: 8\n"
                             ".AA...\n" ".BB...\n");
  const std::string expect[] = {".BB...", ".AA..."};
  std::istringstream iss(book_str);

  PatternForTest pattern;
  pattern.ParseBook(iss);

  EXPECT_EQ("Test", pattern.name());
  EXPECT_EQ(1000, pattern.score());
  EXPECT_EQ(8, pattern.max_puyos_);
  for (int x = 0; x < FieldConstant::WIDTH; ++x) {
    for (int y = 0; y < 2; ++y) {
      EXPECT_EQ(expect[y][x], pattern.pattern_[y][x]);
    }
  }
}

TEST_F(PatternTest, DISABLED_ReadBook) {
  const std::string book_str("NAME: Test\n" "SCORE: 1000\n" "MAX: 8\n"
                             ".AA...\n" ".BB...\n----\n");
  std::istringstream iss(book_str + book_str + book_str);

  Pattern::ReadBook(iss);
  const std::vector<Pattern>& patterns = Pattern::GetAllPattern();
  EXPECT_EQ(static_cast<size_t>(3), patterns.size());
  for (auto pat : patterns) {
    EXPECT_EQ("Test", pat.name());
    EXPECT_EQ(1000, pat.score());
  }
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
