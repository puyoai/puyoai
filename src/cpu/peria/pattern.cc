#include "pattern.h"

#include <cctype>
#include <deque>
#include <istream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <gflags/gflags.h>

#include "core/rensa/rensa_detector.h"
#include "core/pattern/decision_book.h"

#include "base.h"
#include "rensa_vanishing_position_tracker.h"

// TODO: Move the book to BOOK_DIR
DEFINE_string(dynamic_pattern, PERIA_ROOT "/dynamic_book.txt", "Figures a template file name.");
DEFINE_string(joseki_book, BOOK_DIR "/joseki.toml", "Figures a file name for Joseki.");

namespace peria {

namespace {

std::string Trim(std::string line) {
  size_t pos = line.find('#');
  // Remove comments starting with '#'.
  if (pos != std::string::npos)
    line = line.substr(0, pos);

  size_t start = line.find_first_not_of(' ');
  size_t last = line.find_last_not_of(' ');
  if (start == std::string::npos)
    start = 0;
  if (last == std::string::npos)
    last = line.size();
  return line.substr(start, last - start + 1);
}

}  // namespace

DecisionBook* Pattern::getJoseki() {
  static std::unique_ptr<DecisionBook> s_joseki(new DecisionBook());
  if (s_joseki && !s_joseki->load(FLAGS_joseki_book)) {
    LOG(INFO) << "Failed to load JOSEKI file: " << FLAGS_joseki_book;
    s_joseki.reset();
  }
  return s_joseki.get();
}

DynamicPatternBook* Pattern::getDynamicPattern() {
  NOTREACHED();
  return nullptr;
}

StaticPatternBook* Pattern::getStaticPattern() {
  NOTREACHED();
  return nullptr;
}

const int DynamicPattern::kDefaultScore = 100;

DynamicPatternBook::Book DynamicPatternBook::book_;

DynamicPattern::DynamicPattern() : score(0) {}

DynamicPattern::DynamicPattern(std::istream& is) : score(kDefaultScore) {
  std::vector<std::string> lines;
  for (std::string line; std::getline(is, line);) {
    line = Trim(line);
    if (line.empty())
      continue;
    if (line.find("--") == 0)
      break;

    std::istringstream iss(line);
    std::string first_segment;
    iss >> first_segment;
    if (first_segment.find(':') == std::string::npos) {
      lines.push_back(first_segment);
      continue;
    }

    if (first_segment == "NAME:") {
      iss >> name;
    } else if (first_segment == "SCORE:") {
      iss >> score;
    }
  }

  if (lines.empty())
    return;

  // Set field information into field.
  std::reverse(lines.begin(), lines.end());
  for (size_t i = 0; i < lines.size(); ++i) {
    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
      if (std::isupper(lines[i][x - 1])) {
        bits.set(x, i + 1);
      }
    }
  }
}

void DynamicPatternBook::readBook(std::istream& is) {
  while (!is.eof()) {
    DynamicPattern pattern(is);
    if (pattern.bits.isEmpty())
      break;
    // TODO: check duplicates
    book_.insert(std::make_pair(pattern.bits, pattern));
  }
}

int DynamicPatternBook::iteratePatterns(const CoreField& field, std::string* best_name) {
  int best_score = 0;
  auto callback = [&best_score, &best_name](CoreField&& field, const ColumnPuyoList&) {
    RensaVanishingPositionTracker tracker;
    RensaResult rensaResult = field.simulate(&tracker);
    const VanishingPositionTrackerResult& result = tracker.result();
    DCHECK_EQ(result.size(), rensaResult.chains);

    std::string name;
    int score = 0;
    for (int i = 1; i <= rensaResult.chains; ++i) {
      FieldBits bits = result.getBasePuyosAt(i);
      DCHECK(!bits.isEmpty());

      auto it = book().find(bits);
      if (it == book().end()) {
        return;
      }

      const DynamicPattern& pattern = it->second;
      score += pattern.score * (i + 1) / 2;
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

StaticPatternBook::Book StaticPatternBook::book_;

StaticPattern::StaticPattern(std::istream& is) : score(0) {
  std::vector<std::string> lines;
  for (std::string line; std::getline(is, line);) {
    line = Trim(line);
    if (line.empty())
      continue;
    if (line.find("--") == 0)
      break;

    std::istringstream iss(line);
    std::string first_segment;
    iss >> first_segment;
    if (first_segment.find(':') == std::string::npos) {
      lines.push_back(first_segment);
      continue;
    }

    if (first_segment == "NAME:") {
      iss >> name;
    } else if (first_segment == "SCORE:") {
      iss >> score;
    }
  }

  if (lines.empty())
    return;

  // Set field information into field.
  std::reverse(lines.begin(), lines.end());
  std::unordered_set<char> used_chars;
  for (const std::string& line : lines) {
    for (char c : line) {
      if (std::isupper(c))
        used_chars.insert(c);
    }
  }
  for (char c : used_chars) {
    FieldBits bit;
    for (size_t i = 0; i < lines.size(); ++i) {
      const std::string& line = lines[i];
      for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        if (line[x - 1] == c) {
          bit.set(x, i + 1);
        }
      }
    }
    bits.push_back(bit);
  }
}

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
  for (const StaticPattern& pattern : book()) {
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
