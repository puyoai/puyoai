#include "pattern.h"

#include <cctype>
#include <deque>
#include <fstream>
#include <istream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <gflags/gflags.h>
#include <toml/toml.h>

#include "core/rensa/rensa_detector.h"
#include "core/pattern/decision_book.h"

#include "base.h"
#include "rensa_vanishing_position_tracker.h"

DEFINE_string(dynamic_pattern_book, BOOK_DIR "/dynamic_book.toml",
              "Figures a template file name.");
DEFINE_string(joseki_book, BOOK_DIR "/joseki.toml",
              "Figures a file name for Joseki.");

namespace peria {

DecisionBook* Pattern::getJoseki() {
  static std::unique_ptr<DecisionBook> s_joseki;
  if (!s_joseki) {
    s_joseki.reset(new DecisionBook());
    if (!s_joseki->load(FLAGS_joseki_book)) {
      LOG(INFO) << "Failed to load JOSEKI file: " << FLAGS_joseki_book;
    }
  }
  return s_joseki.get();
}

DynamicPatternBook* Pattern::getDynamicPattern() {
  static std::unique_ptr<DynamicPatternBook> s_dynamic;
  if (!s_dynamic) {
    s_dynamic.reset(new DynamicPatternBook());
    if (!s_dynamic->load(FLAGS_dynamic_pattern_book)) {
      LOG(INFO) << "Failed to load dynamic template file: " << FLAGS_dynamic_pattern_book;
    }
  }
  return s_dynamic.get();
}

StaticPatternBook* Pattern::getStaticPattern() {
  NOTREACHED();
  return nullptr;
}

const int DynamicPattern::kDefaultScore = 100;

DynamicPattern::DynamicPattern(const toml::Value& pattern) : score(kDefaultScore) {
  CHECK(pattern.valid());

  // field
  std::vector<std::string> f;
  for (const auto& s : pattern.get<toml::Array>("field")) {
    f.push_back(s.as<std::string>());
  }
  std::reverse(f.begin(), f.end());
  for (size_t i = 0; i < f.size(); ++i) {
    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
      if (std::isupper(f[i][x - 1])) {
        bits.set(x, i + 1);
      }
    }
  }

  if (const toml::Value* p = pattern.find("name")) {
    name = p->as<std::string>();
  }
  if (const toml::Value* p = pattern.find("score")) {
    score = p->as<int>();
  }
}

bool DynamicPatternBook::load(const std::string& filename) {
  std::ifstream ifs(filename);
  toml::ParseResult result = toml::parse(ifs);
  if (!result.valid()) {
    LOG(ERROR) << result.errorReason;
    return false;
  }
  return loadFromValue(std::move(result.value));
}

bool DynamicPatternBook::loadFromString(const std::string& str) {
  std::istringstream iss(str);
  toml::ParseResult result = toml::parse(iss);
  if (!result.valid()) {
    LOG(ERROR) << result.errorReason;
    return false;
  }
  return loadFromValue(std::move(result.value));
}

bool DynamicPatternBook::loadFromValue(const toml::Value& book) {
  const toml::Array& patterns = book.find("pattern")->as<toml::Array>();
  for (const toml::Value& entry : patterns) {
    DynamicPattern pattern(entry);
    // TODO: check duplicates
    book_.insert(std::make_pair(pattern.bits, pattern));
  }
  return true;
}

int DynamicPatternBook::iteratePatterns(const CoreField& field, std::string* best_name) {
  int best_score = 0;
  const Book& book = book_;
  auto callback = [&book, &best_score, &best_name](CoreField&& field, const ColumnPuyoList&) {
    RensaVanishingPositionTracker tracker;
    RensaResult rensaResult = field.simulate(&tracker);
    const VanishingPositionTrackerResult& result = tracker.result();
    DCHECK_EQ(result.size(), rensaResult.chains);

    std::string name;
    int score = 0;
    for (int i = 1; i <= rensaResult.chains; ++i) {
      FieldBits bits = result.getBasePuyosAt(i);
      DCHECK(!bits.isEmpty());

      auto it = book.find(bits);
      if (it == book.end()) {
        return;
      }

      const DynamicPattern& pattern = it->second;
      score += pattern.score;
      name += pattern.name;
    }

    if (score > best_score) {
      best_score = score;
      *best_name = name;
    }
  };
  RensaDetector::detectSingle(field, RensaDetectorStrategy::defaultDropStrategy(), callback);

  return best_score;
}

// ----------------------------------------------------------------------------

StaticPattern::StaticPattern(std::istream&) {}

int StaticPattern::match(const CoreField& field) const {
  int matched_bits = 0;
  const BitField& bit_field = field.bitField();
  for (PuyoColor col : NORMAL_PUYO_COLORS) {
    FieldBits color_bits = bit_field.bits(col);
    for (const FieldBits& bit : bits) {
      FieldBits b = bit & color_bits;
      if (b.isEmpty())
        continue;
      if (b != bit)
        return -1;
      ++matched_bits;
    }
  }
  return score * matched_bits / bits.size();
}

void StaticPatternBook::readBook(std::istream& is) {
  while (!is.eof()) {
    StaticPattern pattern(is);
    if (pattern.bits.size())
      break;
    book_.push_back(pattern);
  }
}

int StaticPatternBook::iteratePatterns(const CoreField& field, std::string* best_name) {
  std::string name;
  int total_score = 0;
  for (const StaticPattern& pattern : book_) {
    int score = pattern.match(field);
    if (score < 0)
      continue;
    total_score += score;
    if (best_name->empty())
      *best_name = pattern.name;
  }
  return total_score;
}

}  // namespace peria
