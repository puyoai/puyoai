#include "field.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <glog/logging.h>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "../../core/constant.h"
#include "base.h"

Field::Field() {
  Init();
}

Field::Field(const string& url) {
  Init();

  string prefix = "http://www.inosendo.com/puyo/rensim/??";
  int data_starts_at = 0;
  if (url.find(prefix) == 0)
    data_starts_at = prefix.length();
  SetField(url.substr(data_starts_at));
}

Field::Field(const Field& f) {
  for (int x = 0; x < kMapWidth; ++x) {
    for (int y = 0; y < kMapHeight; ++y)
      field_[x][y] = f.field_[x][y];
  }
  for (int i = 0; i < kMapWidth; ++i)
    min_heights[i] = f.min_heights[i];
  erased_ = f.erased_;
  zenkeshi_ = f.zenkeshi_;
}

void Field::CopyFrom(const Field& field) {
  for (int x = 1; x <= kWidth; ++x)
    for (int y = 1; y <= kHeight + 2; ++y)
      field_[x][y] = field.field_[x][y];
  erased_ = field.erased_;
  zenkeshi_ = field.zenkeshi_;
}

void Field::SetField(const string& field) {
  for (int i = field.size() - 1, p = 0; i >= 0; --i, ++p) {
    int x = (p + 6 - field.size() % 6) % 6 + 1;
    int y = i / 6 + 1;
    char color = kEmpty;
    switch(field[p]) {
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

void Field::Init() {
  // Initialize state.
  erased_ = false;
  zenkeshi_ = false;

  // Initialize field information.
  for (int x = 0; x < kMapWidth; ++x) {
    for (int y = 0; y < kMapHeight; ++y)
      field_[x][y] = kEmpty;
  }
  for (int x = 0; x < kMapWidth; ++x)
    field_[x][0] = field_[x][kMapHeight - 1] = kWall;
  for (int y = 0; y < kMapHeight; ++y)
    field_[0][y] = field_[kMapWidth - 1][y] = kWall;

  for (int i = 0; i < kMapWidth; ++i)
    min_heights[i] = 100;
}

void Field::Set(int x, int y, char color) {
  if (y > kMapHeight)
    return;

  if (color == kEmpty) {
    erased_ = true;
    color |= kMaskChecked;
  }
  field_[x][y] = color;

  if (y < min_heights[x])
    min_heights[x] = y;
}

void Field::Put(int x, int y_pivot, int r, const string& puyos) {
  const int dx[] = {0, 1, 0, -1};
  const int dy[] = {1, 0, 1, 0};

  for (int y = 1; y <= y_pivot; ++y) {
    if (field_[x][y] != kEmpty)
      continue;
    Set(x, y, puyos[(r == 2) ? 1 : 0]);
    x += dx[r];
    y += dy[r];
    Set(x, y, puyos[(r == 2) ? 0 : 1]);
    break;
  }
  Drop();
}

char Field::Get(int x, int y) const {
  return field_[x][y] & (kMaskChecked - 1);
}

bool Field::IsEmpty(int x, int y) const {
  if (x < 0 || x >= kMapWidth) return false;
  if (y < 0 || y >= kMapHeight) return false;
  return Get(x, y) == kEmpty;
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

const int kLongBonus[] = {
  0, 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 10,
};
static const int kChainBonus[] = {
  0,   0,   8,  16,  32,  64,  96, 128, 160, 192,
  224, 256, 288, 320, 352, 384, 416, 448, 480, 512,
};
static const int kColorBonus[] = {
  0, 0, 3, 6, 12, 24,
};

int GetLongBonus(int length) {
  if (length >= ARRAYSIZE(kLongBonus))
    length = ARRAYSIZE(kLongBonus);
  return kLongBonus[length];
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
      int color = field_[x][y];
      // No puyos above.
      if (color == kEmpty)
        break;
      // This cell is already checked.
      if ((color & kMaskChecked) || color == kOjama)
        continue;
      field_[x][y] |= kMaskChecked;

      write_head[0] = x;
      write_head[1] = y;
      write_head += 2;
      for (; read_head != write_head; read_head += 2) {
        const int rx = read_head[0], ry = read_head[1];
        CheckCell(color, field_, &write_head, rx + 1, ry);
        CheckCell(color, field_, &write_head, rx - 1, ry);
        CheckCell(color, field_, &write_head, rx, ry + 1);
        CheckCell(color, field_, &write_head, rx, ry - 1);
      }

      int num_checked_puyo = (read_head - prev_head) / 2;
      if (num_checked_puyo < kEraseNum) {
        read_head = write_head = prev_head;
      } else {
        bonus += GetLongBonus(num_checked_puyo);
        if (color < kColors && !used_colors[color]) {
          ++num_colors;
          used_colors[color] = 1;
        }
        prev_head = read_head;
      }
    }
  }

  bonus += kColorBonus[num_colors];
  bonus += kChainBonus[chains];
  bonus = (bonus == 0) ? 1 : ((bonus > 999) ? 999 : bonus);
  int erased_puyos = (read_head - erase_field) / 2;
  erased_ = (erased_puyos > 0);
  *score += 10 * erased_puyos * bonus;
  if (zenkeshi_ && erased_) {
    *score += ZENKESHI_BONUS;
    zenkeshi_ = false;
  }

  // Actually erase the Puyos to be vanished.
  for (int i = 1; i <= kWidth; ++i)
    min_heights[i] = 100;

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

void Field::Drop() {
  int frames;
  Drop(&frames);
}

void Field::Drop(int* frames) {
  if (!erased_)
    return;

  int max_drops = 0;
  for (int x = 1; x <= kWidth; ++x) {
    int write_at = min_heights[x];
    for (int y = write_at + 1; y < kMapHeight - 1; ++y) {
      if (field_[x][y] == kEmpty)
        break;
      if (field_[x][y] != (kEmpty | kMaskChecked)) {
        if (y - write_at > max_drops)
          max_drops = y - write_at;
        field_[x][write_at] = field_[x][y] & (kMaskChecked - 1);
        field_[x][y] = kEmpty | kMaskChecked;
        write_at++;
      }
    }
  }
  Clean_();
  erased_ = false;

  if (max_drops == 0)
    *frames += FRAMES_AFTER_NO_DROP;
  else
    *frames += FRAMES_DROP_1_LINE * max_drops + FRAMES_AFTER_DROP;
}

void Field::Clean_() {
  for (int x = 1; x <= kWidth; ++x) {
    for (int y = 1; y <= kHeight + 2; ++y) {
      if (field_[x][y] == kEmpty)
        break;
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

string Field::GetDebugOutput() const {
  ostringstream s;
  static char char_map[256] = {};
  for (int i = 0; i < 256; ++i)
    char_map[i] = '?';
  char_map[kWall]   = char_map[kWall   | kMaskChecked] = '#';
  char_map[kOjama]  = char_map[kOjama  | kMaskChecked] = '@';
  char_map[kRed]    = char_map[kRed    | kMaskChecked] = 'R';
  char_map[kGreen]  = char_map[kGreen  | kMaskChecked] = 'G';
  char_map[kBlue]   = char_map[kBlue   | kMaskChecked] = 'B';
  char_map[kYellow] = char_map[kYellow | kMaskChecked] = 'Y';
  char_map[kEmpty]  = char_map[kEmpty  | kMaskChecked] = ' ';

  for (int y = kMapHeight - 1; y >= 0; y--) {
    for (int x = 0; x < kMapWidth; ++x)
      s << char_map[field_[x][y]] << " ";
    s << "\n";
  }
  s << "  ";
  for (int x = 1; x <= kWidth; ++x)
    s << min_heights[x] << " ";
  s << "\n";
  return s.str();
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

double Field::Evaluate() {
  return 0;
}
