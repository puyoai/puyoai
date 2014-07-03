#include "field.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include <glog/logging.h>

#include "core/constant.h"
#include "ctrl.h"

// If this flag is turned on, we don't need to check the cell for vanishment
// anymore.
static const int MASK_CHECKED = 0x80;

using namespace std;

void LF::Init() {
  // Initialize field information.
  for (int x = 0; x < LF::MAP_WIDTH; x++) {
    for (int y = 0; y < LF::MAP_HEIGHT; y++) {
      field[x][y] = EMPTY;
    }
  }
  for (int x = 0; x < LF::MAP_WIDTH; x++) {
    field[x][0] = field[x][MAP_HEIGHT - 1] = WALL;
  }
  for (int y = 0; y < LF::MAP_HEIGHT; y++) {
    field[0][y] = field[MAP_WIDTH - 1][y] = WALL;
  }

  for (int i = 0; i < MAP_WIDTH; i++) {
    min_heights[i] = 100;
  }
}

LF::LF() {
  Init();
}

LF::LF(const std::string& url) {
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

void LF::Set(int x, int y, char color) {
  field[x][y] = color;
  if (y < min_heights[x]) {
    min_heights[x] = y;
  }
}

byte LF::Get(int x, int y) const {
  return field[x][y] & (MASK_CHECKED - 1);
}

static int getLongBonus(int length) {
  if (length >= 11) {
    length = 11;
  }
  return LONG_BONUS[length];
}

inline void check_cell(unsigned char color, unsigned char field[][16],
                       int** writer, int x, int y) {
  // If MASK_CHECKED is there, the cell is already checked for deletion.
  if (y <= 12 && color == field[x][y]) {
    (*writer)[0] = x;
    (*writer)[1] = y;
    *writer += 2;
    field[x][y] |= MASK_CHECKED;
  }
}

bool LF::Vanish(int chains, int* score) {
  int erase_field[WIDTH * HEIGHT * 2];
  int* read_head = erase_field;
  int* write_head = erase_field;
  int* prev_head = erase_field;

  int used_colors[COLORS + 1] = {0};
  int num_colors = 0;
  int bonus = 0;

  for (int x = 1; x <= LF::WIDTH; x++) {
    for (int y = min_heights[x]; y <= LF::HEIGHT; y++) {
      // No puyos above.
      if (field[x][y] == EMPTY) {
        break;
      }
      // This cell is already checked.
      if (field[x][y] & MASK_CHECKED) {
        continue;
      }
      if (field[x][y] == OJAMA) {
        continue;
      }

      write_head[0] = x;
      write_head[1] = y;
      write_head += 2;
      char color = field[x][y];
      field[x][y] |= MASK_CHECKED;

      while (read_head != write_head) {
        int x = read_head[0];
        int y = read_head[1];
        read_head += 2;
        check_cell(color, field, &write_head, x + 1, y);
        check_cell(color, field, &write_head, x - 1, y);
        check_cell(color, field, &write_head, x, y + 1);
        check_cell(color, field, &write_head, x, y - 1);
      }
      if (read_head - prev_head < ERASE_NUM * 2) {
        read_head = write_head = prev_head;
      } else {
        bonus += getLongBonus((read_head - prev_head) >> 1);
        if (!used_colors[(int)color]) {
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

  bool erased = false;
  if (erase_field == read_head) {
    erased = false;
  } else {
    erased = true;

    int* head = erase_field;
    while (head < read_head) {
      int x = head[0];
      int y = head[1];
      field[x][y] = EMPTY | MASK_CHECKED;
      if (y < min_heights[x]) {
        min_heights[x] = y;
      }

      if (field[x+1][y] == OJAMA) {
        field[x+1][y] = EMPTY | MASK_CHECKED;
        if (y < min_heights[x + 1]) {
          min_heights[x + 1] = y;
        }
      }
      if (field[x-1][y] == OJAMA) {
        field[x-1][y] = EMPTY | MASK_CHECKED;
        if (y < min_heights[x - 1]) {
          min_heights[x - 1] = y;
        }
      }
      if (field[x][y+1] == OJAMA && y + 1 <= 12) {
        field[x][y+1] = EMPTY | MASK_CHECKED;
      }
      if (field[x][y-1] == OJAMA) {
        field[x][y-1] = EMPTY | MASK_CHECKED;
        if (y - 1 < min_heights[x]) {
          min_heights[x] = y - 1;
        }
      }

      head += 2;
    }
  }

  *score += 10 * erased_puyos * bonus;
  return erased;
}

// Adds frames it takes to the argument "int* frames".
void LF::Drop(int* frames) {
  int max_drops = 0;
  for (int x = 1; x <= LF::WIDTH; x++) {
    int write_at = min_heights[x];
    // Puyo in 14th row won't drop to 13th row. It is a well known bug in Puyo2.
    // Puyos in 14th row can't fall, so they'll stay there forever.
    for (int y = write_at + 1; y < LF::MAP_HEIGHT - 2; y++) {
      if (field[x][y] == EMPTY) {
        break;
      }
      if (field[x][y] != (EMPTY | MASK_CHECKED)) {
        if (y - write_at > max_drops) {
          max_drops = y - write_at;
        }
        field[x][write_at] = field[x][y] & (MASK_CHECKED - 1);
        field[x][y] = EMPTY | MASK_CHECKED;
        write_at++;
      }
    }
  }
  Clean_();

  if (max_drops == 0) {
    *frames += FRAMES_AFTER_NO_DROP;
  } else {
    *frames += FRAMES_DROP_1_LINE * max_drops + FRAMES_AFTER_DROP;
  }
}

void LF::Drop() {
  int frames;
  Drop(&frames);
}

void LF::SafeDrop() {
  for (int x = 1; x <= LF::WIDTH; x++) {
    for (int y = 1; y < LF::MAP_HEIGHT - 1; y++) {
      if (field[x][y] && !field[x][y-1]) {
        int ny = y - 1;
        while (!field[x][ny-1])
          ny--;
        field[x][ny] = field[x][y];
        field[x][y] = EMPTY;
      }
    }
  }
}

void LF::Clean_() {
  for (int x = 1; x <= LF::WIDTH; x++) {
    for (int y = 1; y <= LF::HEIGHT + 2; y++) {
      if (field[x][y] == EMPTY) {
        break;
      }
      field[x][y] &= (MASK_CHECKED - 1);
    }
  }
}

void LF::Simulate() {
  int score, chain, frames;
  Simulate(&score, &chain, &frames);
}

void LF::Simulate(int* chains, int* score, int* frames) {
  SimulateFromChain(1, chains, score, frames);
}

void LF::SimulateFromChain(int init_chains,
                           int* chains, int* score, int* frames) {
  *score = 0;
  *chains = init_chains;
  *frames = 0;
  while (Vanish(*chains, score)) {
    *frames += FRAMES_AFTER_VANISH + FRAMES_VANISH_ANIMATION;
    Drop(frames);
    (*chains)++;
  }
  Clean_();
  (*chains)--;
}

static char getPuyoChar(char c) {
  switch (c & 0x3f) {
  case OJAMA:
    return '@';
  case RED:
    return 'R';
  case BLUE:
    return 'B';
  case YELLOW:
    return 'Y';
  case GREEN:
    return 'G';
  case EMPTY:
    return ' ';
  case WALL:
    return '#';
  default:
    return '?';
  }
}

const string LF::GetDebugOutput() const {
  std::ostringstream s;
  for (int y = MAP_HEIGHT - 1; y >= 0; y--) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      s << getPuyoChar(field[x][y]) << " ";
    }
    s << std::endl;
  }
  s << "  ";
  for (int x = 1; x <= WIDTH; x++) {
    s << (int)min_heights[x] << " ";
  }
  s << '\n';
  s << url();
  s << '\n';
  return s.str();
}

const string LF::GetDebugOutput(const string& next) const {
  string s(GetDebugOutput());
  s += "YOKOKU=";
  s += GetDebugOutputForNext(next);
  s += '\n';
  return s;
}

const string LF::GetDebugOutputForNext(const string& next) {
  string s;
  for (int i = 0; i < 6; i++) {
    s += getPuyoChar(next[i]);
  }
  return s;
}

void LF::FindAvailablePlans(const string& next, int depth, vector<LP>* plans) {
  if (depth <= 0 || 4 <= depth) {
    LOG(ERROR) << "depth parameter shoube be 1, 2 or 3.";
  }
  plans->clear();
  plans->reserve(22 + 22*22 + 22*22*22);
  FindAvailablePlansInternal(*this, next, NULL, 0, depth, plans);
}

void LF::FindAvailablePlans(const string& next, vector<LP>* plans) {
  FindAvailablePlans(next, 3, plans);
}

static const Decision all_decisions[22] = {
  Decision(1, 2),
  Decision(2, 2),
  Decision(3, 2),
  Decision(4, 2),
  Decision(5, 2),
  Decision(6, 2),
  Decision(2, 3),
  Decision(3, 3),
  Decision(4, 3),
  Decision(5, 3),
  Decision(6, 3),

  Decision(1, 0),
  Decision(2, 0),
  Decision(3, 0),
  Decision(4, 0),
  Decision(5, 0),
  Decision(6, 0),
  Decision(1, 1),
  Decision(2, 1),
  Decision(3, 1),
  Decision(4, 1),
  Decision(5, 1),
};

void LF::FindAvailablePlansInternal(const LF& field, const string& next, const LP* parent, int depth, int max_depth, vector<LP>* plans) {
  char c1 = next[depth * 2 + 0];
  char c2 = next[depth * 2 + 1];
  int num_decisions;
  const Decision* decisions;
  if (c1 == c2) {
    num_decisions = 11;
    decisions = &all_decisions[11];
  } else {
    num_decisions = 22;
    decisions = &all_decisions[0];
  }

  // Cause of slowness?
  int heights[LF::MAP_WIDTH+1];
  for (int x = 1; x <= LF::WIDTH; x++) {
    heights[x] = 100;
    for (int y = 1; y <= LF::HEIGHT + 2; y++) {
      if (field.Get(x, y) == EMPTY) {
        heights[x] = y;
        break;
      }
    }
  }

  for (int i = 0; i < num_decisions; i++) {
    const Decision& decision = decisions[i];
    if (!LC::isReachable(field, decision)) {
      continue;
    }

    LF next_field(field);

    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    int chigiri_frames = FRAMES_AFTER_CHIGIRI + abs(x1 - x2) / 2;

    if (decision.r == 2) {
      next_field.Set(x2, heights[x2]++, c2);
      next_field.Set(x1, heights[x1]++, c1);
    } else {
      next_field.Set(x1, heights[x1]++, c1);
      next_field.Set(x2, heights[x2]++, c2);
    }
    heights[x1]--;
    heights[x2]--;
    int chains, score, frames;
    next_field.Simulate(&chains, &score, &frames);
    if (next_field.Get(3, 12) != EMPTY) {
      continue;
    }

    // Add a new plan.
    plans->push_back(LP());
    LP& plan = plans->at(plans->size() - 1);
    plan.field = next_field;
    plan.decision = decision;
    plan.parent = parent;
    if (parent) {
      plan.score = parent->score + score;
      plan.chain_cnt = parent->chain_cnt + chains;
      plan.chigiri_frames = parent->chigiri_frames + chigiri_frames;
    } else {
      plan.score = score;
      plan.chain_cnt = chains;
      plan.chigiri_frames = chigiri_frames;
    }

    if (depth < max_depth - 1) {
      FindAvailablePlansInternal(next_field, next, &plan, depth + 1, max_depth, plans);
    }
  }
}

int LF::PutDecision(Decision decision, char c1, char c2, int* chigiri_frames) {
  // Cause of slowness?
  int heights[LF::MAP_WIDTH];
  for (int x = 1; x <= LF::WIDTH; x++) {
    heights[x] = 100;
    for (int y = 1; y <= LF::HEIGHT + 2; y++) {
      if (Get(x, y) == EMPTY) {
        heights[x] = y;
        break;
      }
    }
  }

  int x1 = decision.x;
  int x2 = decision.x + (decision.r == 1) - (decision.r == 3);
  if (chigiri_frames && heights[x1] != heights[x2]) {
    int d = abs(heights[x1] - heights[x2]);
    // Wow, this looks a precise approximation.
    *chigiri_frames += FRAMES_AFTER_CHIGIRI + d / 2;
  }
  if (decision.r == 2) {
    Set(x2, heights[x2]++, c2);
    Set(x1, heights[x1]++, c1);
  } else {
    Set(x1, heights[x1]++, c1);
    Set(x2, heights[x2]++, c2);
  }

  int chains, score, frames;
  Simulate(&chains, &score, &frames);
  if (Get(3, 12) != EMPTY) {
    return -1;
  }

  return score;
}

const string LF::query_string() const {
  string q;
  for (int y = 13; y >= 1; y--) {
    for (int x = 1; x <= 6; x++) {
      if (Get(x, y) || !q.empty()) {
        char c = '?';
        switch (Get(x, y)) {
        case OJAMA:
          c = '1'; break;
        case RED:
          c = '4'; break;
        case BLUE:
          c = '5'; break;
        case YELLOW:
          c = '6'; break;
        case GREEN:
          c = '7'; break;
        case EMPTY:
          c = '0'; break;
        case 8:
          // for purple in parse_movie
          c = '8'; break;
        default:
          LOG(ERROR) << (int)field[x][y];
        }
        q.push_back(c);
      }
    }
  }
  return q;
}

const string LF::url() const {
  return "http://www.inosendo.com/puyo/rensim/?" + query_string();
}

int LF::countPuyo() const {
  int n = 0;
  for (int x = 1; x <= 6; x++) {
    for (int y = 1; y <= 13; y++) {
      if (!Get(x, y))
        break;
      n++;
    }
  }
  return n;
}

int LF::countColorPuyo() const {
  int n = 0;
  for (int x = 1; x <= 6; x++) {
    for (int y = 1; y <= 13; y++) {
      char c = Get(x, y);
      if (!c)
        break;
      if (c >= RED || c <= GREEN + 1)
        n++;
    }
  }
  return n;
}

bool LF::hasPuyo() const {
  for (int x = 1; x <= 6; x++) {
    if (Get(x, 1))
      return true;
  }
  return false;
}

Decision LP::getFirstDecision() const {
  const LP* p = this;
  while (p->parent)
    p = p->parent;
  Decision d = p->decision;
  return d;
}

int LF::getBestChainCount(int* ignition_puyo_cnt,
                          int* useful_chain_cnt_out,
                          int* vanished_puyo_cnt) const {
  int best_chain = 1;
  int useful_chain_cnt = 0;
  int puyo_cnt_before = countColorPuyo();

  if (ignition_puyo_cnt) {
    *ignition_puyo_cnt = 0;
  }
  if (vanished_puyo_cnt) {
    *vanished_puyo_cnt = 0;
  }

  bool has_checked[7][16];
  memset(has_checked, 0, sizeof(has_checked));

  for (int x = 1; x <= 6; x++) {
    for (int y = 1; y < 13; y++) {
      if (has_checked[x][y])
        continue;
      char c = Get(x, y);
      if (!c)
        break;
      if (c < RED)
        continue;
      if (Get(x + 1, y) && Get(x - 1, y) && Get(x, y + 1))
        continue;

      LF nf(*this);
      for (int i = 1; i <= 6; i++) {
        nf.min_heights[i] = 100;
      }
      int n = 0;
#define SET_EMPTY(x, y)                         \
      nf.Set(x, y, EMPTY | MASK_CHECKED);       \
      n++;                                      \
      has_checked[x][y] = true;

      SET_EMPTY(x, y);
      if (nf.Get(x + 1, y) == c) {
        SET_EMPTY(x + 1, y);
        if (nf.Get(x + 2, y) == c) {
          SET_EMPTY(x + 2, y);
        } else if (nf.Get(x + 1, y + 1) == c) {
          SET_EMPTY(x + 1, y + 1);
        } else if (nf.Get(x + 1, y - 1) == c) {
          SET_EMPTY(x + 1, y - 1);
        }
      }
      if (nf.Get(x - 1, y) == c) {
        SET_EMPTY(x - 1, y);
        if (nf.Get(x - 2, y) == c) {
          SET_EMPTY(x - 2, y);
        } else if (nf.Get(x - 1, y + 1) == c) {
          SET_EMPTY(x - 1, y + 1);
        } else if (nf.Get(x - 1, y - 1) == c) {
          SET_EMPTY(x - 1, y - 1);
        }
      }
      if (nf.Get(x, y + 1) == c) {
        SET_EMPTY(x, y + 1);
        if (nf.Get(x, y + 2) == c) {
          SET_EMPTY(x, y + 2);
        } else if (nf.Get(x + 1, y + 1) == c) {
          SET_EMPTY(x + 1, y + 1);
        } else if (nf.Get(x - 1, y + 1) == c) {
          SET_EMPTY(x - 1, y + 1);
        }
      }
      if (nf.Get(x, y - 1) == c) {
        SET_EMPTY(x, y - 1);
        if (nf.Get(x, y - 2) == c) {
          SET_EMPTY(x, y - 2);
        } else if (nf.Get(x + 1, y - 1) == c) {
          SET_EMPTY(x + 1, y - 1);
        } else if (nf.Get(x - 1, y - 1) == c) {
          SET_EMPTY(x - 1, y - 1);
        }
      }
#undef SET_EMPTY

      int chain, score, frame;
      nf.Drop();
      nf.SimulateFromChain(2, &chain, &score, &frame);
      if (best_chain < chain) {
        best_chain = chain;
        if (ignition_puyo_cnt) {
          *ignition_puyo_cnt = n;
        }
        if (vanished_puyo_cnt) {
          *vanished_puyo_cnt = puyo_cnt_before - n - nf.countColorPuyo();
        }
      } else if (best_chain == chain) {
        if (ignition_puyo_cnt) {
          *ignition_puyo_cnt += n;
        }
        if (vanished_puyo_cnt) {
          *vanished_puyo_cnt = min(*vanished_puyo_cnt,
                                   puyo_cnt_before - n - nf.countColorPuyo());
        }
      }
      if (score > 840) {
        useful_chain_cnt++;
      }
    }
  }
  if (useful_chain_cnt_out)
    *useful_chain_cnt_out = useful_chain_cnt;
  return best_chain;
}

int LF::getOjamaFilmHeight(int* hidden_color_puyo_cnt) const {
  int h = 100;
  int ojama_max_y = 12;
  int ojama_min_y = 2;
  int cnt = 0;
  for (int x = 1; x <= 6; x++) {
    int next_ojama_max_y = 0;
    for (int y = ojama_max_y + 1; y >= ojama_min_y - 1; y--) {
      if (field[x][y] == OJAMA) {
        next_ojama_max_y = y;
        break;
      }
    }
    if (!next_ojama_max_y)
      return 0;

    int next_ojama_min_y = next_ojama_max_y;
    while (field[x][next_ojama_min_y-1] == OJAMA)
      next_ojama_min_y--;

    h = min(h, next_ojama_max_y - max(ojama_min_y - 1, next_ojama_min_y) + 1);

    ojama_max_y = next_ojama_max_y;
    ojama_min_y = next_ojama_min_y;

    for (int y = ojama_min_y - 1; y >= 1; y--) {
      if (field[x][y] >= RED)
        cnt++;
    }
  }

  if (hidden_color_puyo_cnt)
    *hidden_color_puyo_cnt = cnt;
  return h;
}

const string LF::parseNext(const char* p) {
  string next;
  parseNext(p, &next);
  return next;
}

void LF::parseNext(const char* p, string* next) {
  next->resize(6);
  for (int i = 0; i < 6; i++) {
    int c = p[i] - '0';
    CHECK_GE(c, 4) << i;
    CHECK_LE(c, 7) << i;
    (*next)[i] = c;
  }
}

bool LF::isEqual(const LF& f) const {
  for (int x = 1; x <= 6; x++) {
    for (int y = 1; y < 13; y++) {
      if (Get(x, y) != f.Get(x, y))
        return false;
    }
  }
  return true;
}

void LF::getProspectiveChains(vector<Chain*>* pchains,
                              int i, int d, int depth) const {
  int cnt = countColorPuyo();
  for (; i < 24; i++) {
    int x = i / 4 + 1;
    int c = i % 4 + 4;
    int y;
    for (y = 1; y <= 13; y++) {
      if (!Get(x, y))
        break;
    }
    if (y > 12)
      continue;

    LF f(*this);
    f.Set(x, y, c);

    if (d < depth) {
      f.getProspectiveChains(pchains, i / 4 * 4, d + 1, depth);
    }

    int chains, score, frames;
    f.Simulate(&chains, &score, &frames);
/*
    LOG(INFO) << "depth=" << depth << " x=" << x << " c=" << c
              << " chains=" << chains << " score=" << score;
*/
    if (chains && score > 420) {
      Chain* chain = new Chain;
      chain->chains = chains;
      chain->score = score;
      chain->frames = frames;
      chain->depth = d;
      chain->vanished = cnt + 1 - f.countColorPuyo();
      pchains->push_back(chain);
      continue;
    }
  }
}

void LF::getProspectiveChains(vector<Chain*>* pchains) const {
  getProspectiveChains(pchains, 0, 1, 3);
  //sort(pchains->begin(), pchains->end(), Chain::ByScore());
  sort(pchains->begin(), pchains->end(), Chain::ByBest());
}

bool LF::complementOjamasDropped(const LF& f) {
  int max_ojama = 0;
  int min_ojama = INT_MAX;
  int num_ojamas[7];
  int heights[7];
  for (int x = 1; x <= 6; x++) {
    num_ojamas[x] = 0;
    heights[x] = 1;
    int n = 0;
    for (int y = 1; y <= 12; heights[x] = ++y) {
      char c = Get(x, y);
      char pc = f.Get(x, y);
      if (c == pc) {
        if (c == EMPTY && pc == EMPTY)
          break;
        continue;
      }
      if (c != OJAMA || pc != EMPTY) {
#if 0
        LOG(WARNING) << "Inconsistent field from the previous field "
                     << (char)(c + '0') << " vs " << (char)(pc + '0')
                     << " @" << x << "," << y << '\n'
                     << f.GetDebugOutput() << "vs\n" << GetDebugOutput();
        break;
#endif
        continue;
      }
      n++;
    }
    num_ojamas[x] = n;
    max_ojama = max(max_ojama, n);
    if (heights[x] < 12)
      min_ojama = min(min_ojama, n);
  }

  if (!max_ojama) {
    return false;
  }

  if (max_ojama > 5) {
    LOG(WARNING) << "Too many ojamas has dropped max_ojama="
                 << max_ojama << '\n'
                 << f.GetDebugOutput() << "vs\n" << GetDebugOutput();
    max_ojama = 5;
  } else if (max_ojama < 5) {
    if (min_ojama == max_ojama) {
      max_ojama++;
    } else if (min_ojama + 1 != max_ojama && min_ojama < max_ojama) {
      LOG(WARNING) << "Inconsistent ojama drop max_ojama="
                   << max_ojama << " min_ojama=" << min_ojama << '\n'
                   << f.GetDebugOutput() << "vs\n" << GetDebugOutput();
    }
  }

  LOG(INFO) << "Guessing the field after the ojama:\n" << GetDebugOutput();

  for (int x = 1; x <= 6; x++) {
    if (heights[x] <= 12)
      continue;
    int n = max_ojama - num_ojamas[x];
    for (int y = heights[x]; y <= 14 && n > 0; y++) {
      char pc = f.Get(x, y);
      if (pc == EMPTY) {
        Set(x, y, OJAMA);
        n--;
      }
    }
  }

  LOG(INFO) << "Guessed the field after the ojama:\n" << GetDebugOutput();
  return true;
}
