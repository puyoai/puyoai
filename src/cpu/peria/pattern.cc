#include "pattern.h"

#include <cctype>
#include <deque>
#include <istream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace peria {

namespace {

std::vector<Pattern> g_patterns;

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

void Pattern::ReadBook(std::istream& is) {
  g_patterns.clear();
  while (!is.eof()) {
    Pattern pattern;
    if (!pattern.ParseBook(is))
      break;
    g_patterns.push_back(pattern);
  }
}

const std::vector<Pattern>& Pattern::GetAllPattern() {
  return g_patterns;
}

bool Pattern::ParseBook(std::istream& is) {
  for (std::string line; std::getline(is, line);) {
    line = Trim(line);
    if (line.empty())
      continue;
    if (line.find("----") == 0)
      break;

    std::istringstream iss(line);
    std::string first_segment;
    iss >> first_segment;
    if (first_segment.find(':') == std::string::npos) {
      AppendField(first_segment);
      continue;
    }

    if (first_segment == "NAME:") {
      iss >> name_;
    } else if (first_segment == "SCORE:") {
      iss >> score_;
    } else if (first_segment == "MAX:") {
      iss >> max_puyos_;
    }
  }

  if (name_.empty())
    return false;

  Optimize();

  return true;
}

int Pattern::Match(const CoreField& field) const {
  if (max_puyos_ > 0 && field.countPuyos() > max_puyos_)
    return 0;

  MatchingCounts matching0;
  MatchingCounts matching1;  // Mirroring
  const int height = pattern_.size();
  for (int y = 1; y <= height; ++y) {
    for (int x = 1; x <= PlainField::WIDTH; ++x) {
      const char c0 = pattern_[y - 1][x - 1];
      const char c1 = pattern_[y - 1][PlainField::WIDTH - x];
      PuyoColor color = field.get(x, y);
      if (color != PuyoColor::OJAMA) {
        if (c0 != '.')
          ++matching0[c0][color];
        if (c1 != '.')
          ++matching1[c1][color];
      }
    }
  }

  int score = std::max(GetScore(matching0), GetScore(matching1));
  return score;
}

void Pattern::Optimize() {
  static const int dx[] = {0, 1};
  static const int dy[] = {1, 0};

  const int height = pattern_.size();
  for (int y0 = 0; y0 < height; ++y0) {
    for (int x0 = 0; x0 < PlainField::WIDTH; ++x0) {
      for (int i = 0; i < 2; ++i) {
        int x1 = x0 + dx[i], y1 = y0 + dy[i];
        if (x1 >= PlainField::WIDTH || y1 >= height)
          continue;
        char c0 = pattern_[y0][x0];
        char c1 = pattern_[y1][x1];
        neighbors_.insert(Neighbor(c0, c1));
        neighbors_.insert(Neighbor(c1, c0));
      }
    }
  }

  num_puyos_ = 0;
  for (const auto& line : pattern_) {
    for (char c : line) {
      if (c != '.')
        ++num_puyos_;
    }
  }
}

void Pattern::AppendField(std::string line) {
  DCHECK(line.size() == PlainField::WIDTH);

  for (auto& c : line) {
    if (std::islower(c))
      c = '.';
  }
  pattern_.push_front(line);
}

int Pattern::GetScore(MatchingCounts& matching) const {
  int sum = 0;
  matching['_'][PuyoColor::EMPTY] += 0;
  if (matching['_'].size() == 1)
    sum = matching['_'][PuyoColor::EMPTY];
  matching.erase('_');

  for (auto& matching_itr : matching) {
    DCHECK(std::isupper(matching_itr.first));
    auto& color_map = matching_itr.second;
    color_map.erase(PuyoColor::EMPTY);
    if (color_map.size() > 1)
      return 0;
    if (!color_map.empty())
      sum += color_map.begin()->second;
  }

  // Neighborhoods cannot be same color.
  for (const auto& neighbor : neighbors_) {
    char c0 = neighbor.first;
    char c1 = neighbor.second;
    if (matching[c0].empty() || matching[c1].empty())
      continue;
    if (matching[c0].begin()->first == matching[c1].begin()->first)
      return 0;
  }

  // TODO: 指定してないセルが指定セルと同じ色を持ってないか確認
  LOG(INFO) << name_ << ": " << score_ << " " << sum << " " << num_puyos_;
  return score_ * sum / num_puyos_;
}

}  // namespace peria
