#include "pattern.h"

#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "core/pattern/decision_book.h"
#include "core/core_field.h"
#include "core/field_bits.h"
#include "core/field_constant.h"
#include "core/puyo_color.h"

namespace peria {

class TestableDynamicPatternBook : public DynamicPatternBook {
 public:
  TestableDynamicPatternBook(const std::string& str) {
    EXPECT_TRUE(loadFromString(str));
  }
  size_t size() { return book_.size(); }
  Book::iterator find(const FieldBits& key) { return book_.find(key); }
  const Book::iterator end() { return book_.end(); }
};

TEST(DynamicPatternTest, parsePattern) {
  const std::string book_str(
      "[[pattern]]\n"
      "name = \"Test\"\n"
      "score = 24\n"
      "field = [\n"
      "    \"AAAA..\"\n"
      "]\n");
  TestableDynamicPatternBook book(book_str);

  EXPECT_EQ(1U, book.size());
  FieldBits expect("1111..");
  auto it = book.find(expect);
  ASSERT_NE(book.end(), it);
  const DynamicPattern& pattern = it->second;
  EXPECT_EQ("Test", pattern.name);
  EXPECT_EQ(24, pattern.score);
  EXPECT_EQ(expect, pattern.bits);
}

TEST(DynamicPatternTest, readBook) {
  const std::string book_str(
      "[[pattern]]\n"
      "name = \"Test\"\n"
      "field = [\n"
      "    \"AAAA..\"\n"
      "]\n"
      "\n"
      "[[pattern]]\n"
      "name = \"Test2\"\n"
      "field = [\n"
      "    \"..A...\",\n"
      "    \".AAA..\"\n"
      "]\n");

  TestableDynamicPatternBook book(book_str);
  EXPECT_EQ(2U, book.size());

  FieldBits no_entry("...1.."
                     "..111.");
  FieldBits entry1("1111..");
  FieldBits entry2("..1..."
                  ".111..");
  EXPECT_EQ(book.end(), book.find(no_entry));
  auto it = book.find(entry1);
  EXPECT_NE(book.end(), it);
  EXPECT_EQ("Test", it->second.name);
  it = book.find(entry2);
  EXPECT_NE(book.end(), it);
  EXPECT_EQ("Test2", it->second.name);
}

TEST(DynamicPatternTest, iteratePatterns) {
  const std::string book_str(
      "[[pattern]]\n"
      "name = \"Test\"\n"
      "field = [\n"
      "    \"AAAA..\"\n"
      "]\n"
      "\n"
      "[[pattern]]\n"
      "field = [\n"
      "    \".AAAA.\"\n"
      "]\n");
  TestableDynamicPatternBook book(book_str);
  ASSERT_EQ(2U, book.size());

  // Assume this field will be  vvvvvv and vanish in 2-chain.
  CoreField field(".RRR.."   // .RRR..
                  "RBBB.."); // RBBBB.
  std::string name;
  int score = book.iteratePatterns(field, &name);
  EXPECT_EQ("Test", name);
  EXPECT_EQ(DynamicPattern::kDefaultScore * 2, score);
}

}  // namespace peria
