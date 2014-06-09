#include "deprecated_field.h"

#include <cstdlib>
#include <glog/logging.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>

#include "core/ctrl.h"
#include "core/decision.h"

using namespace std;

// If this flag is turned on, we don't need to check the cell for vanishment
// anymore.
static const int MASK_CHECKED = 0x80;

void DeprecatedField::Init() {
  // Initialize state.
  erased_ = false;

  // Initialize field information.
  for (int x = 0; x < DeprecatedField::MAP_WIDTH; x++) {
    for (int y = 0; y < DeprecatedField::MAP_HEIGHT; y++) {
      field_[x][y] = EMPTY;
    }
  }
  for (int x = 0; x < DeprecatedField::MAP_WIDTH; x++) {
    field_[x][0] = field_[x][MAP_HEIGHT - 1] = WALL;
  }
  for (int y = 0; y < DeprecatedField::MAP_HEIGHT; y++) {
    field_[0][y] = field_[MAP_WIDTH - 1][y] = WALL;
  }

  for (int i = 0; i < MAP_WIDTH; i++) {
    min_heights[i] = 100;
  }
}

DeprecatedField::DeprecatedField() {
  Init();
}

DeprecatedField::DeprecatedField(const std::string& url) {
  Init();

  std::string prefix = "http://www.inosendo.com/puyo/rensim/??";
  int data_starts_at;
  if (url.find(prefix) == 0) {
    data_starts_at = prefix.length();
  } else {
    data_starts_at = 0;
  }

  int counter = 0;
  for (int i = url.length() - 1; i >= data_starts_at; i--) {
    int x = 6 - (counter % 6);
    int y = counter / 6 + 1;
    char color = EMPTY;
    switch(url[i]) {
      case '0': color = EMPTY; break;
      case '1': color = OJAMA; break;
      case '2': color = WALL; break;
      case '4': color = RED; break;
      case '5': color = BLUE; break;
      case '6': color = YELLOW; break;
      case '7': color = GREEN; break;
      default: break;
    }
    Set(x, y, color);
    counter++;
  }
}

DeprecatedField::DeprecatedField(const DeprecatedField& f) {
  for (int x = 0; x < DeprecatedField::MAP_WIDTH; x++) {
    for (int y = 0; y < DeprecatedField::MAP_HEIGHT; y++) {
      field_[x][y] = f.field_[x][y];
    }
  }
  for (int i = 0; i < MAP_WIDTH; i++) {
    min_heights[i] = f.min_heights[i];
  }
  erased_ = f.erased_;
}

void DeprecatedField::Set(int x, int y, char color) {
  if (color == EMPTY) {
    erased_ = true;
    field_[x][y] = color | MASK_CHECKED;
  } else {
    field_[x][y] = color;
  }
  if (y < min_heights[x]) {
    min_heights[x] = y;
  }
}

char DeprecatedField::Get(int x, int y) const {
  return field_[x][y] & (MASK_CHECKED - 1);
}

int getLongBonus(int length) {
  if (length >= 11) {
    length = 11;
  }
  return LONG_BONUS[length];
}

inline void check_cell(unsigned char color, unsigned char field_[][16], int[] /* min_heights */,
                       int** writer, int x, int y) {
  // If MASK_CHECKED is there, the cell is already checked for deletion.
  if (y <= 12 && color == field_[x][y]) {
    (*writer)[0] = x;
    (*writer)[1] = y;
    *writer += 2;
    field_[x][y] |= MASK_CHECKED;
  }
}

bool DeprecatedField::Vanish(int chains, int* score) {
  int erase_field[WIDTH * HEIGHT * 2];
  int* read_head = erase_field;
  int* write_head = erase_field;
  int* prev_head = erase_field;

  int used_colors[PUYO_COLORS + 1] = {0};
  int num_colors = 0;
  int bonus = 0;

  for (int x = 1; x <= DeprecatedField::WIDTH; x++) {
    for (int y = min_heights[x]; y <= DeprecatedField::HEIGHT; y++) {
      // No puyos above.
      if (field_[x][y] == EMPTY) {
        break;
      }
      // This cell is already checked.
      if (field_[x][y] & MASK_CHECKED) {
        continue;
      }
      if (field_[x][y] == OJAMA) {
        continue;
      }

      write_head[0] = x;
      write_head[1] = y;
      write_head += 2;
      char color = field_[x][y];
      field_[x][y] |= MASK_CHECKED;

      while (read_head != write_head) {
        int x = read_head[0];
        int y = read_head[1];
        read_head += 2;
        check_cell(color, field_, min_heights, &write_head, x + 1, y);
        check_cell(color, field_, min_heights, &write_head, x - 1, y);
        check_cell(color, field_, min_heights, &write_head, x, y + 1);
        check_cell(color, field_, min_heights, &write_head, x, y - 1);
      }
      if (read_head - prev_head < PUYO_ERASE_NUM * 2) {
        read_head = write_head = prev_head;
      } else {
        bonus += getLongBonus((read_head - prev_head) >> 1);
        // Some AI uses colors >=COLORS for simulation.
        // Such colors are ignored in used_colors.
        if (color < PUYO_COLORS && !used_colors[(int)color]) {
          num_colors++;
          used_colors[(int)color] = 1;
        }
        prev_head = read_head;
      }
    }
  }

  bonus += COLOR_BONUS[num_colors];
  bonus += CHAIN_BONUS[chains];
  // We need to add rakka-bonus etc.
  // http://puyora.ktkr.net/puyozan.html
  int erased_puyos = (read_head - erase_field) >> 1;
  if (bonus == 0) {
    bonus = 1;
  }
  if (bonus > 999) {
    bonus = 999;
  }

  // Actually erase the Puyos to be vanished.
  for (int i = 1; i <= WIDTH; i++) {
    min_heights[i] = 100;
  }

  if (erase_field == read_head) {
    erased_ = false;
  } else {
    erased_ = true;

    int* head = erase_field;
    while (head < read_head) {
      int x = head[0];
      int y = head[1];
      field_[x][y] = EMPTY | MASK_CHECKED;
      if (y < min_heights[x]) {
        min_heights[x] = y;
      }

      if (field_[x+1][y] == OJAMA) {
        field_[x+1][y] = EMPTY | MASK_CHECKED;
        if (y < min_heights[x + 1]) {
          min_heights[x + 1] = y;
        }
      }
      if (field_[x-1][y] == OJAMA) {
        field_[x-1][y] = EMPTY | MASK_CHECKED;
        if (y < min_heights[x - 1]) {
          min_heights[x - 1] = y;
        }
      }
      if (field_[x][y+1] == OJAMA && y + 1 <= 12) {
        field_[x][y+1] = EMPTY | MASK_CHECKED;
      }
      if (field_[x][y-1] == OJAMA) {
        field_[x][y-1] = EMPTY | MASK_CHECKED;
        if (y - 1 < min_heights[x]) {
          min_heights[x] = y - 1;
        }
      }

      head += 2;
    }
  }

  *score += 10 * erased_puyos * bonus;
  return erased_;
}

// Adds frames it takes to the argument "int* frames".
void DeprecatedField::Drop(int* frames) {
  if (!erased_) {
    return;
  }

  int max_drops = 0;
  for (int x = 1; x <= DeprecatedField::WIDTH; x++) {
    int write_at = min_heights[x];
    // Puyo in 14th row won't drop to 13th row. It is a well known bug in Puyo2.
    // Puyos in 14th row can't fall, so they'll stay there forever.
    for (int y = write_at + 1; y < DeprecatedField::MAP_HEIGHT - 2; y++) {
      if (field_[x][y] == EMPTY) {
        break;
      }
      if (field_[x][y] != (EMPTY | MASK_CHECKED)) {
        if (y - write_at > max_drops) {
          max_drops = y - write_at;
        }
        field_[x][write_at] = field_[x][y] & (MASK_CHECKED - 1);
        field_[x][y] = EMPTY | MASK_CHECKED;
        write_at++;
      }
    }
  }
  Clean();
  erased_ = false;

  if (max_drops == 0) {
    *frames += FRAMES_AFTER_NO_DROP;
  } else {
    *frames += FRAMES_DROP_1_LINE * max_drops + FRAMES_AFTER_DROP;
  }
}

void DeprecatedField::Drop() {
  int frames;
  Drop(&frames);
}

void DeprecatedField::Clean() {
  for (int x = 1; x <= DeprecatedField::WIDTH; x++) {
    for (int y = 1; y <= DeprecatedField::HEIGHT + 2; y++) {
      if (field_[x][y] == EMPTY) {
        break;
      }
      field_[x][y] &= (MASK_CHECKED - 1);
    }
  }
}

void DeprecatedField::Simulate() {
  int score, chain, frames;
  Simulate(&score, &chain, &frames);
}

void DeprecatedField::Simulate(int* chains, int* score, int* frames) {
  *score = 0;
  *chains = 1;
  *frames = 0;
  while (Vanish(*chains, score)) {
    *frames += FRAMES_AFTER_VANISH;
    Drop(frames);
    (*chains)++;
  }
  Clean();
  (*chains)--;
}

std::string DeprecatedField::GetDebugOutput() const {
  std::ostringstream s;
  static char char_map[256] = {};
  for (int i = 0; i < 256; i++) {
    char_map[i] = '?';
  }
  char_map[WALL] = '#';
  char_map[OJAMA] = '@';
  char_map[RED] = 'R';
  char_map[GREEN] = 'G';
  char_map[BLUE] = 'B';
  char_map[YELLOW] = 'Y';
  char_map[EMPTY] = ' ';

  for (int y = MAP_HEIGHT - 1; y >= 0; y--) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      s << char_map[field_[x][y] & 127] << " ";
    }
    s << std::endl;
  }
  s << "  ";
  for (int x = 1; x <= WIDTH; x++) {
    s << min_heights[x] << " ";
  }
  s << std::endl;
  return s.str();
}

void FieldWithColorSequence::SetColorSequence(const string& sequence) {
  color_sequence_ = sequence;
  for (int i = 0; i < int(color_sequence_.size()); i++) {
    color_sequence_[i] -= '0';
  }
  next_puyo_ = 0;
}

std::string FieldWithColorSequence::GetColorSequence() const {
  string sequence = color_sequence_;
  for (int i = 0; i < int(sequence.size()); i++) {
    sequence[i] += '0';
  }
  return sequence;
}

char FieldWithColorSequence::GetNextPuyo(int n) const {
  assert(!color_sequence_.empty());
  int len = color_sequence_.length();
  if (len == 0) {
    LOG(FATAL) << "You must call DeprecatedField::SetColorSequence() before calling"
               << "DeprecatedField::FindAvailablePlansInternal()";
  }
  return color_sequence_[(next_puyo_ + n) % len];
}

string FieldWithColorSequence::GetDebugOutput() const {
  std::ostringstream s;

  s << "YOKOKU=";
  for (int i = 0; i < (int)color_sequence_.size(); i++) {
    s << (char)('0' + color_sequence_[i]);
  }
  s << std::endl;

  return DeprecatedField::GetDebugOutput() + s.str();
}
