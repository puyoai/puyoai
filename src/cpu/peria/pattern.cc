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

void Pattern::BuildCombination() {
  // TODO: Do not combine a template with a disabled flag.
  int n = g_patterns.size();
  for (int i = 0; i < n; ++i) {
    for (int j = i + 1; j < n; ++j) {
      Pattern pattern(g_patterns[i]);
      if (pattern.MergeWith(g_patterns[j]))
        g_patterns.push_back(pattern);
    }
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
    } else if (first_segment == "DIFF:") {
      char c0, c1;
      iss >> c0 >> c1;
      neighbors_.insert(Neighbor(c0, c1));
    }
  }

  if (name_.empty())
    return false;

  Optimize();

  return true;
}

int Pattern::Match(const CoreField& field) const {
  MatchingCounts matching0;
  MatchingCounts matching1;  // Mirroring
  const int height = pattern_.size();
  for (int y = 1; y <= height; ++y) {
    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
      const char c0 = pattern_[y - 1][x - 1];
      const char c1 = pattern_[y - 1][FieldConstant::WIDTH - x];
      PuyoColor color = field.color(x, y);
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

bool Pattern::MergeWith(const Pattern& a) {
  static const char kNonDetermLine[] = "......";
  while (pattern_.size() < a.pattern_.size())
    pattern_.push_back(kNonDetermLine);

  // TODO: If conflicted characters can be same
  // we should merge them.
  int offset = 0;
  for (const std::string& l : pattern_) {
    for (char c : l) {
      if (isupper(c) && c > offset)
        offset = c;
    }
  }
  if (isupper(offset))
    offset = offset - 'A' + 1;
  else
    offset = 0;

  bool merge = true;
  for (size_t i = 0; i < a.pattern_.size(); ++i) {
    for (int j = 0; j < FieldConstant::WIDTH; ++j) {
      if (a.pattern_[i][j] == '.')
        continue;

      if (pattern_[i][j] == '.')
        pattern_[i][j] = a.pattern_[i][j] + offset;
      else
        merge = false;
    }
  }

  if (!merge)
    return false; // There are conflicts

  name_ += "+" + a.name_;
  score_ += a.score_ - 50;
  for (auto& n : a.neighbors_) {
    neighbors_.insert(Neighbor(n.first + offset, n.second + offset));
  }
  num_puyos_ += a.num_puyos_;
  Optimize();
  
  return true;
}

void Pattern::Optimize() {
  static const int dx[] = {0, 1};
  static const int dy[] = {1, 0};

  const int height = pattern_.size();
  for (int y0 = 0; y0 < height; ++y0) {
    for (int x0 = 0; x0 < FieldConstant::WIDTH; ++x0) {
      for (int i = 0; i < 2; ++i) {
        int x1 = x0 + dx[i], y1 = y0 + dy[i];
        if (x1 >= FieldConstant::WIDTH || y1 >= height)
          continue;
        char c0 = pattern_[y0][x0];
        char c1 = pattern_[y1][x1];
        if (c0 == c1)
          continue;
        neighbors_.insert(Neighbor(c0, c1));
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
  DCHECK(line.size() == FieldConstant::WIDTH);

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
