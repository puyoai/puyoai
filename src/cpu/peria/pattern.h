#pragma once

#include <deque>
#include <istream>
#include <set>
#include <string>
#include <unordered_map>

#include <toml/toml.h>

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
  explicit DynamicPattern(const toml::Value&);

  // TODO: |bits| is a key in PatternBook, so we don't need to keep it here
  FieldBits bits;
  std::string name;
  int score;

  static const int kDefaultScore;
};

class DynamicPatternBook {
 public:
  bool load(const std::string& filename);
  bool loadFromString(const std::string& str);
  bool loadFromValue(const toml::Value& book);

  // Simulate possible rensas, and find matching patterns
  int iteratePatterns(const CoreField& field, std::string* name);

  size_t size() { return book_.size(); }

 protected:
  using Book = std::unordered_map<FieldBits, DynamicPattern>;
  Book book_;
};

// StaticPattern class figures where non-vanishing puyos are.
class StaticPattern {
 public:
  explicit StaticPattern(std::istream&);

  int match(const CoreField& field) const;

  std::vector<FieldBits> bits;
  std::string name;
  int score;
};

class StaticPatternBook {
 public:
  void readBook(std::istream& is);

  // Simulate possible rensas, and find matching patterns
  int iteratePatterns(const CoreField& field, std::string* name);

 private:
  using Book = std::vector<StaticPattern>;
  Book book_;
};

}  // namespace peria
