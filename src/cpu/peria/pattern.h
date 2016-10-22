#pragma once

#include <deque>
#include <istream>
#include <set>
#include <string>
#include <unordered_map>

#include "core/core_field.h"
#include "core/field_bits.h"

class DecisionBook;

namespace peria {

class DynamicPatternBook;
class StaticPatternBook;

class Pattern {
 public:
  static DecisionBook* getJoseki();
  static DynamicPatternBook* getDynamicPattern();
  static StaticPatternBook* getStaticPattern();
};

// DynamicPattern class figures where puyos are being vanished
class DynamicPattern {
 public:
  DynamicPattern();
  explicit DynamicPattern(std::istream& is);

  // TODO: |bits| is a key in PatternBook, so we don't need to keep it here
  FieldBits bits;
  std::string name;
  int score;

  static const int kDefaultScore;
};

class DynamicPatternBook {
 public:
  using Book = std::unordered_map<FieldBits, DynamicPattern>;

  static void readBook(std::istream& is);
  static void clear() { book_.clear(); }

  // Simulate possible rensas, and find matching patterns
  static int iteratePatterns(const CoreField& field, std::string* name);

 protected:
  static const Book& book() { return book_; }

 private:
  static Book book_;
};

// StaticPattern class figures where non-vanishing puyos are.
class StaticPattern {
 public:
  explicit StaticPattern(std::istream& is);

  int match(const CoreField& field) const;

  std::vector<FieldBits> bits;
  std::string name;
  int score;
};

class StaticPatternBook {
 public:
  using Book = std::vector<StaticPattern>;

  static void readBook(std::istream& is);
  static void clear() { book_.clear(); }

  // Simulate possible rensas, and find matching patterns
  static int iteratePatterns(const CoreField& field, std::string* name);

 protected:
  static const Book& book() { return book_; }

 private:
  static Book book_;
};

}  // namespace peria
