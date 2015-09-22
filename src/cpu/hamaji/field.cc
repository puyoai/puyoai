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

#include "core/rensa/rensa_detector.h"
#include "core/column_puyo_list.h"
#include "core/kumipuyo_seq.h"
#include "core/puyo_controller.h"
#include "core/rensa_result.h"

using namespace std;

void LF::Simulate() {
  int score, chain, frames;
  Simulate(&score, &chain, &frames);
}

void LF::Simulate(int* chains, int* score, int* frames) {
  SimulateFromChain(1, chains, score, frames);
}

void LF::SimulateFromChain(int init_chains,
                           int* chains, int* score, int* frames) {
  RensaResult result = simulate(init_chains);
  *score = result.score;
  *chains = result.chains;
  *frames = result.frames;
}

const string LF::GetDebugOutput() const {
  return toDebugString();
}

void LF::FindAvailablePlans(const KumipuyoSeq& next, int depth, vector<LP>* plans) {
  if (depth <= 0 || 4 <= depth) {
    LOG(ERROR) << "depth parameter shoube be 1, 2 or 3.";
  }
  plans->clear();
  plans->reserve(22 + 22*22 + 22*22*22);
  FindAvailablePlansInternal(*this, next, NULL, 0, depth, plans);
}

void LF::FindAvailablePlans(const KumipuyoSeq& next, vector<LP>* plans) {
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

void LF::FindAvailablePlansInternal(const LF& field, const KumipuyoSeq& next, const LP* parent, int depth, int max_depth, vector<LP>* plans) {
  PuyoColor c1 = next.axis(depth);
  PuyoColor c2 = next.child(depth);
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
      if (field.Get(x, y) == PuyoColor::EMPTY) {
        heights[x] = y;
        break;
      }
    }
  }

  for (int i = 0; i < num_decisions; i++) {
    const Decision& decision = decisions[i];
    if (!PuyoController::isReachable(field, decision)) {
      continue;
    }

    LF next_field(field);

    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    int chigiri_frames = FRAMES_TO_DROP[abs(x1 - x2)] + FRAMES_GROUNDING;

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
    if (next_field.Get(3, 12) != PuyoColor::EMPTY) {
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

int LF::PutDecision(Decision decision, PuyoColor c1, PuyoColor c2, int* chigiri_frames) {
  // Cause of slowness?
  int heights[LF::MAP_WIDTH];
  for (int x = 1; x <= LF::WIDTH; x++) {
    heights[x] = 100;
    for (int y = 1; y <= LF::HEIGHT + 2; y++) {
      if (Get(x, y) == PuyoColor::EMPTY) {
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
    *chigiri_frames += FRAMES_GROUNDING + FRAMES_TO_DROP[d];
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
  if (Get(3, 12) != PuyoColor::EMPTY) {
    return -1;
  }

  return score;
}

int LF::countColorPuyo() const {
  int n = 0;
  for (int x = 1; x <= 6; x++) {
    for (int y = 1; y <= 13; y++) {
      if (isEmpty(x, y))
        break;
      if (isNormalColor(x, y))
        n++;
    }
  }
  return n;
}

bool LF::hasPuyo() const {
  for (int x = 1; x <= 6; x++) {
    if (Get(x, 1) != PuyoColor::EMPTY)
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

  if (ignition_puyo_cnt)
    *ignition_puyo_cnt = 0;
  if (vanished_puyo_cnt)
    *vanished_puyo_cnt = 0;

  auto callback = [&](CoreField&& nf, const ColumnPuyoList& cpl) {
    RensaResult rensa_result = nf.simulate();
    int chain = rensa_result.chains;
    int n = 4 - cpl.size();
    if (n < 0)
      n = 0;

    if (best_chain < chain) {
      best_chain = chain;
      if (ignition_puyo_cnt) {
        *ignition_puyo_cnt = n;
      }
      if (vanished_puyo_cnt) {
        *vanished_puyo_cnt = puyo_cnt_before - n - nf.countColorPuyos();
      }
    } else if (best_chain == chain) {
      if (ignition_puyo_cnt) {
        *ignition_puyo_cnt = std::max(n, *ignition_puyo_cnt);
      }
      if (vanished_puyo_cnt) {
        *vanished_puyo_cnt = min(*vanished_puyo_cnt,
                                 puyo_cnt_before - n - nf.countColorPuyos());
      }
    }
    if (rensa_result.score > 840) {
      useful_chain_cnt++;
    }
  };
  RensaDetector::detectSingle(*this, RensaDetectorStrategy::defaultFloatStrategy(), callback);

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
      if (Get(x, y) == PuyoColor::OJAMA) {
        next_ojama_max_y = y;
        break;
      }
    }
    if (!next_ojama_max_y)
      return 0;

    int next_ojama_min_y = next_ojama_max_y;
    while (Get(x, next_ojama_min_y-1) == PuyoColor::OJAMA)
      next_ojama_min_y--;

    h = min(h, next_ojama_max_y - max(ojama_min_y - 1, next_ojama_min_y) + 1);

    ojama_max_y = next_ojama_max_y;
    ojama_min_y = next_ojama_min_y;

    for (int y = ojama_min_y - 1; y >= 1; y--) {
      if (Get(x, y) >= PuyoColor::RED)
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
      if (Get(x, y) == PuyoColor::EMPTY)
        break;
    }
    if (y > 12)
      continue;

    LF f(*this);
    f.Set(x, y, toPuyoColor(c));

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
      PuyoColor c = Get(x, y);
      PuyoColor pc = f.Get(x, y);
      if (c == pc) {
        if (c == PuyoColor::EMPTY && pc == PuyoColor::EMPTY)
          break;
        continue;
      }
      if (c != PuyoColor::OJAMA || pc != PuyoColor::EMPTY) {
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
      PuyoColor pc = f.Get(x, y);
      if (pc == PuyoColor::EMPTY) {
        Set(x, y, PuyoColor::OJAMA);
        n--;
      }
    }
  }

  LOG(INFO) << "Guessed the field after the ojama:\n" << GetDebugOutput();
  return true;
}

string LF::query_string() const {
  string q;
  for (int y = 13; y >= 1; y--) {
    for (int x = 1; x <= 6; x++) {
      if (color(x, y) != PuyoColor::EMPTY || !q.empty()) {
        char c = '?';
        switch (color(x, y)) {
        case PuyoColor::OJAMA:
          c = '1'; break;
        case PuyoColor::RED:
          c = '4'; break;
        case PuyoColor::BLUE:
          c = '5'; break;
        case PuyoColor::YELLOW:
          c = '6'; break;
        case PuyoColor::GREEN:
          c = '7'; break;
        case PuyoColor::EMPTY:
          c = '0'; break;
        default:
          LOG(ERROR) << toChar(color(x, y));
        }
        q.push_back(c);
      }
    }
  }

  return q;
}
