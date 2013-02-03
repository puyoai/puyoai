#include "field.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <glog/logging.h>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "base.h"
#include "decision.h"

char Field::GetNextPuyo(int n) const {
  assert(!color_sequence_.empty());
  int len = color_sequence_.length();
  if (len == 0) {
    LOG(FATAL) << "You must call Field::SetColorSequence() before calling"
               << "Field::FindAvailablePlansInternal()";
  }
  return color_sequence_[(next_puyo_ + n) % len];
}

void Field::Init() {
  // Initialize state.
  erased_ = false;
  zenkeshi_ = false;

  // Initialize field information.
  for (int x = 0; x < kMapWidth; ++x) {
    for (int y = 0; y < kMapHeight; ++y) {
      field_[x][y] = kEmpty;
    }
  }
  for (int x = 0; x < kMapWidth; ++x) {
    field_[x][0] = field_[x][kMapHeight - 1] = kWall;
  }
  for (int y = 0; y < kMapHeight; ++y) {
    field_[0][y] = field_[kMapWidth - 1][y] = kWall;
  }

  for (int i = 0; i < kMapWidth; ++i) {
    min_heights[i] = 100;
  }
}

Field::Field() {
  Init();
}

void Field::SetField(const string& field) {
  for (size_t i = 0; i < field.size(); ++i) {
    int x = i % 6 + 1;
    int y = i / 6 + 1;
    char color = kEmpty;
    switch(field[i]) {
      case '0': color = kEmpty; break;
      case '1': color = kOjama; break;
      case '2': color = kWall; break;
      case '4': color = kRed; break;
      case '5': color = kBlue; break;
      case '6': color = kYellow; break;
      case '7': color = kGreen; break;
      default: break;
    }
    Set(x, y, color);
  }
}

Field::Field(const string& url) {
  Init();

  std::string prefix = "http://www.inosendo.com/puyo/rensim/??";
  int data_starts_at = 0;
  if (url.find(prefix) == 0) {
    data_starts_at = prefix.length();
  }
  SetField(url.substr(data_starts_at));
}

Field::Field(const Field& f) {
  for (int x = 0; x < kMapWidth; ++x) {
    for (int y = 0; y < kMapHeight; ++y) {
      field_[x][y] = f.field_[x][y];
    }
  }
  for (int i = 0; i < kMapWidth; ++i) {
    min_heights[i] = f.min_heights[i];
  }
  erased_ = f.erased_;
  zenkeshi_ = f.zenkeshi_;
}

void Field::SetColorSequence(const std::string& sequence) {
  color_sequence_ = sequence;
  for (int i = 0; i < int(color_sequence_.size()); ++i) {
    color_sequence_[i] -= '0';
  }
  next_puyo_ = 0;
}

std::string Field::GetColorSequence() const {
  std::string sequence = color_sequence_;
  for (int i = 0; i < int(sequence.size()); ++i) {
    sequence[i] += '0';
  }
  return sequence;
}

void Field::Set(int x, int y, char color) {
  if (y > kMapHeight)
    return;

  if (color == kEmpty) {
    erased_ = true;
    color = color | kMaskChecked;
  }
  field_[x][y] = color;

  if (y < min_heights[x]) {
    min_heights[x] = y;
  }
}

char Field::Get(int x, int y) const {
  return field_[x][y] & (kMaskChecked - 1);
}

int getLongBonus(int length) {
  if (length >= 11) {
    length = 11;
  }
  return LONG_BONUS[length];
}

namespace {
inline void CheckCell(unsigned char color,
		      unsigned char field_[][Field::kMapHeight],
		      int** writer, int x, int y) {
  // If kMaskChecked is there, the cell is already checked for deletion.
  if (y <= 12 && color == field_[x][y]) {
    (*writer)[0] = x;
    (*writer)[1] = y;
    *writer += 2;
    field_[x][y] |= kMaskChecked;
  }
}
}  // namespace

bool Field::Vanish(int chains, int* score) {
  static int erase_field[kWidth * kHeight * 2];
  int* read_head = erase_field;
  int* write_head = erase_field;
  int* prev_head = erase_field;

  int used_colors[kColors + 1] = {0};
  int num_colors = 0;
  int bonus = 0;

  for (int x = 1; x <= kWidth; ++x) {
    for (int y = min_heights[x]; y <= kHeight; ++y) {
      // No puyos above.
      if (field_[x][y] == kEmpty)
        break;
      // This cell is already checked.
      if (field_[x][y] & kMaskChecked || field_[x][y] == kOjama)
        continue;

      char color = field_[x][y];
      field_[x][y] |= kMaskChecked;

      write_head[0] = x;
      write_head[1] = y;
      write_head += 2;
      for (; read_head != write_head; read_head += 2) {
        const int x = read_head[0], y = read_head[1];
        CheckCell(color, field_, &write_head, x + 1, y);
        CheckCell(color, field_, &write_head, x - 1, y);
        CheckCell(color, field_, &write_head, x, y + 1);
        CheckCell(color, field_, &write_head, x, y - 1);
      }

      if (read_head - prev_head < kEraseNum * 2) {
        read_head = write_head = prev_head;
      } else {
        bonus += getLongBonus((read_head - prev_head) / 2);
        int c = color;
        if (color < kColors && !used_colors[c]) {
          ++num_colors;
          used_colors[c] = 1;
        }
        prev_head = read_head;
      }
    }
  }

  bonus += COLOR_BONUS[num_colors];
  bonus += CHAIN_BONUS[chains];
  int erased_puyos = (read_head - erase_field) / 2;
  if (bonus == 0)
    bonus = 1;
  if (bonus > 999)
    bonus = 999;
  *score += 10 * erased_puyos * bonus;

  // Actually erase the Puyos to be vanished.
  for (int i = 1; i <= kWidth; ++i)
    min_heights[i] = 100;

  if (erase_field == read_head) {
    erased_ = false;
  } else {
    erased_ = true;

    for (int* head = erase_field; head < read_head; head += 2) {
      int x = head[0];
      int y = head[1];
      field_[x][y] = kEmpty | kMaskChecked;
      if (y < min_heights[x])
        min_heights[x] = y;

      if (field_[x + 1][y] == kOjama) {
        field_[x + 1][y] = kEmpty | kMaskChecked;
        if (y < min_heights[x + 1])
          min_heights[x + 1] = y;
      }
      if (field_[x - 1][y] == kOjama) {
        field_[x - 1][y] = kEmpty | kMaskChecked;
        if (y < min_heights[x - 1])
          min_heights[x - 1] = y;
      }
      if (field_[x][y - 1] == kOjama) {
        field_[x][y - 1] = kEmpty | kMaskChecked;
        if (y - 1 < min_heights[x])
          min_heights[x] = y - 1;
      }
      if (field_[x][y + 1] == kOjama && y + 1 <= 12)
        field_[x][y + 1] = kEmpty | kMaskChecked;
    }
  }

  return erased_;
}

bool Field::Vanishable(int x, int y) {
  static int erase_field[kWidth * kHeight * 2];

  // No puyos on (x, y).
  if (field_[x][y] == kEmpty ||
      (field_[x][y] & kMaskChecked) ||
      field_[x][y] == kOjama) {
    return false;
  }

  char color = field_[x][y];
  field_[x][y] |= kMaskChecked;

  int* write_head = erase_field;
  write_head[0] = x;
  write_head[1] = y;
  write_head += 2;

  int* read_head = erase_field;
  for (read_head = erase_field; read_head != write_head; read_head += 2) {
    const int x = read_head[0], y = read_head[1];
    CheckCell(color, field_, &write_head, x + 1, y);
    CheckCell(color, field_, &write_head, x - 1, y);
    CheckCell(color, field_, &write_head, x, y + 1);
    CheckCell(color, field_, &write_head, x, y - 1);
  }
  // Clean up check bits
  for (int* head = erase_field; head < read_head; head += 2) {
    const int x = head[0], y = head[1];
    field_[x][y] &= (kMaskChecked - 1);
  }
  
  return (read_head - erase_field >= kEraseNum * 2);
}

void Field::Drop(int* frames) {
  if (!erased_) {
    return;
  }

  int max_drops = 0;
  for (int x = 1; x <= kWidth; ++x) {
    int write_at = min_heights[x];
    for (int y = write_at + 1; y < kMapHeight - 1; ++y) {
      if (field_[x][y] == kEmpty) {
        break;
      }
      if (field_[x][y] != (kEmpty | kMaskChecked)) {
        if (y - write_at > max_drops) {
          max_drops = y - write_at;
        }
        field_[x][write_at] = field_[x][y] & (kMaskChecked - 1);
        field_[x][y] = kEmpty | kMaskChecked;
        write_at++;
      }
    }
  }
  Clean_();
  erased_ = false;

  if (max_drops == 0) {
    *frames += FRAMES_AFTER_NO_DROP;
  } else {
    *frames += FRAMES_DROP_1_LINE * max_drops + FRAMES_AFTER_DROP;
  }
}

void Field::Drop() {
  int frames;
  Drop(&frames);
}

void Field::Clean_() {
  for (int x = 1; x <= kWidth; ++x) {
    for (int y = 1; y <= kHeight + 2; ++y) {
      if (field_[x][y] == kEmpty) {
        break;
      }
      field_[x][y] &= (kMaskChecked - 1);
    }
  }
}

void Field::Simulate() {
  int score = 0, chain = 1, frames = 0;
  Simulate(&score, &chain, &frames);
}

void Field::Simulate(int* chains, int* score, int* frames) {
  *score = 0;
  *frames = 0;
  while (Vanish(*chains, score)) {
    *frames += FRAMES_AFTER_VANISH;
    Drop(frames);
    (*chains)++;
  }
  Clean_();
  (*chains)--;
}

std::string Field::GetDebugOutput() const {
  std::ostringstream s;
  static char char_map[256] = {};
  for (int i = 0; i < 256; ++i) {
    char_map[i] = '?';
  }
  char_map[kWall] = '#';
  char_map[kOjama] = '@';
  char_map[kRed] = 'R';
  char_map[kGreen] = 'G';
  char_map[kBlue] = 'B';
  char_map[kYellow] = 'Y';
  char_map[kEmpty] = ' ';

  for (int y = kMapHeight - 1; y >= 0; y--) {
    for (int x = 0; x < kMapWidth; ++x) {
      s << char_map[field_[x][y] & 127] << " ";
    }
    s << std::endl;
  }
  s << "  ";
  for (int x = 1; x <= kWidth; ++x) {
    s << min_heights[x] << " ";
  }
  s << std::endl;
  s << "YOKOKU=";
  for (int i = 0; i < 6; ++i) {
    s << (char)('0' + color_sequence_[i]);
  }
  s << std::endl;
  return s.str();
}

namespace {
typedef pair<int, int> Dec;
typedef pair<Dec, int> KumiPos;

inline void add(int x, int y, int r, set<KumiPos>* st, queue<KumiPos>* q) {
  KumiPos p(Dec(x, r), y);
  if (st->find(p) == st->end()) {
    st->insert(p);
    q->push(p);
  }
}

}  // namespace

void Field::FindAvailableControls(bool same, vector<Decision>* decisions) {
  set<Dec> decs;
  set<KumiPos> st;
  queue<KumiPos> q;

  decisions->clear();

  // Dead check
  if (Get(kWidth / 2, kHeight) != kEmpty) {
    return;
  }

  add(kWidth / 2, kHeight, 0, &st, &q);
  // List up all available controls.
  for (; !q.empty(); q.pop()) {
    KumiPos& pos = q.front();
    const Dec& dec = pos.first;
    const int x = dec.first, r = dec.second, y = pos.second;
    if (!same || r < 2)
      decs.insert(Dec(x, r));

    switch (r) {
    case 0:
      // Move right
      if (Get(x + 1, y) == kEmpty && Get(x + 1, y + 1) == kEmpty) {
        add(x + 1, y, 0, &st, &q);
      }
      // Move left
      if (Get(x - 1, y) == kEmpty && Get(x - 1, y + 1) == kEmpty) {
        add(x - 1, y, 0, &st, &q);
      }
      // Turn right
      if (Get(x + 1, y) == kEmpty) {
        add(x, y, 1, &st, &q);
      } else if (Get(x - 1, y) == kEmpty) {
        add(x - 1, y, 1, &st, &q);
      } else if (y + 1 < kHeight + 2) {
        add(x, y + 1, 2, &st, &q);  // Quick turn
      }
      // Turn left
      if (Get(x - 1, y) == kEmpty) {
        add(x, y, 3, &st, &q);
      } else if (Get(x + 1, y) == kEmpty) {
        add(x + 1, y, 3, &st, &q);
      }
      break;
    case 1:
      // Move right
      if (Get(x + 2, y) == kEmpty) {
        add(x + 1, y, 1, &st, &q);
      }
      // Move left
      if (Get(x - 1, y) == kEmpty) {
        add(x - 1, y, 1, &st, &q);
      }
      // Turn right
      if (Get(x, y - 1) == kEmpty) {
        add(x, y, 2, &st, &q);
      } else if (Get(x, y + 1) == kEmpty && y + 1 < kHeight + 2) {
        add(x, y + 1, 2, &st, &q);
      }
      // Turn left
      if (Get(x, y + 1) == kEmpty) {
        add(x, y, 0, &st, &q);
      } else if(Get(x, y - 1) == kEmpty) {
        add(x, y - 1, 0, &st, &q);
      }
      break;
    case 2:
      // Move right
      if (Get(x + 1, y) == kEmpty && Get(x + 1, y - 1) == kEmpty) {
        add(x + 1, y, 2, &st, &q);
      }
      // Move left
      if (Get(x - 1, y) == kEmpty && Get(x - 1, y - 1) == kEmpty) {
        add(x - 1, y, 2, &st, &q);
      }
      // Turn right
      if (Get(x - 1, y) == kEmpty) {
        add(x, y, 3, &st, &q);
      } else if (Get(x + 1, y) == kEmpty) {
        add(x + 1, y, 3, &st, &q);
      } else {
        add(x, y - 1, 0, &st, &q);  // Quick turn
      }
      // Turn left
      if (Get(x + 1, y) == kEmpty) {
        add(x, y, 1, &st, &q);
      } else if (Get(x - 1, y) == kEmpty) {
        add(x - 1, y, 1, &st, &q);
      }
      break;
    case 3:
      // Move right
      if (Get(x + 1, y) == kEmpty) {
        add(x + 1, y, 3, &st, &q);
      }
      // Move left
      if (Get(x - 2, y) == kEmpty) {
        add(x - 1, y, 3, &st, &q);
      }
      // Turn right
      if (Get(x, y + 1) == kEmpty) {
        add(x, y, 0, &st, &q);
      } else if (Get(x, y - 1) == kEmpty) {
        add(x, y - 1, 0, &st, &q);
      }
      // Turn left
      if (Get(x, y - 1) == kEmpty) {
        add(x, y, 2, &st, &q);
      } else if (Get(x, y + 1) == kEmpty && y + 1 < kHeight + 2) {
        add(x, y + 1, 2, &st, &q);
      }
      break;
    }
  }

  for (set<Dec>::iterator itr = decs.begin(); itr != decs.end(); ++itr) {
    decisions->push_back(Decision(itr->first, itr->second));
  }
}

bool Field::EqualTo(const Field& field, bool visible) const {
  const int height = visible ? kHeight : (kHeight + 2);
  for (int x = 1; x <= kWidth; ++x) {
    for (int y = 1; y <= height; ++y) {
      if (Get(x, y) != field.Get(x, y))
	return false;
    }
  }
  return true;
}
