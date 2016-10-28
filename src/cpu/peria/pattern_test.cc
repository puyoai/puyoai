#include "pattern.h"

#include "core/pattern/decision_book.h"
#include "core/core_field.h"
#include "core/field_bits.h"
#include "core/field_constant.h"
#include "core/puyo_color.h"

#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace peria {

class DynamicPatternBookForTest : public DynamicPatternBook {
 public:
  using DynamicPatternBook::book;
};

class DynamicPatternTest : public testing::Test {
 public:
  void SetUp() override {
    DynamicPatternBook::clear();
  }
  void TearDown() override {
    DynamicPatternBook::clear();
  }

  const DynamicPatternBook::Book& book() {
    return DynamicPatternBookForTest::book();
  }
};

TEST_F(DynamicPatternTest, parsePattern) {
  const std::string book_str("NAME: Test\n"
                             "SCORE: 24\n"
                             "AAAA..\n");
  std::istringstream iss(book_str);
  DynamicPattern pattern(iss);;

  FieldBits expect("1111..");
  EXPECT_EQ("Test", pattern.name);
  EXPECT_EQ(24, pattern.score);
  EXPECT_EQ(expect, pattern.bits);
}

TEST_F(DynamicPatternTest, readBook) {
  const std::string book_str("NAME: Test\n"
                             "AAAA..\n"
                             "----\n"
                             "NAME: Test2\n"
                             "..A...\n"
                             ".AAA..\n");
  std::istringstream iss(book_str);

  DynamicPatternBook::readBook(iss);
  EXPECT_EQ(2U, book().size());

  FieldBits no_entry("...1.."
                     "..111.");
  FieldBits entry1("1111..");
  FieldBits entry2("..1..."
                  ".111..");
  EXPECT_EQ(book().end(), book().find(no_entry));
  auto it = book().find(entry1);
  EXPECT_NE(book().end(), it);
  EXPECT_EQ("Test", it->second.name);
  it = book().find(entry2);
  EXPECT_NE(book().end(), it);
  EXPECT_EQ("Test2", it->second.name);
}

TEST_F(DynamicPatternTest, iteratePatterns) {
  const std::string book_str("NAME: Test\n"
                             "AAAA..\n"
                             "----\n"
                             ".AAAA.\n");
  std::istringstream iss(book_str);
  DynamicPatternBook::readBook(iss);
  ASSERT_EQ(2U, book().size());

  // Assume this field will be  vvvvvv and vanish in 2-chain.
  CoreField field(".RRR.."   // .RRR..
                  "RBBB.."); // RBBBB.
  std::string name;
  int score = DynamicPatternBook::iteratePatterns(field, &name);
  EXPECT_EQ("Test", name);
  EXPECT_EQ(DynamicPattern::kDefaultScore * 2, score);
}

}  // namespace peria
