#include "commentator.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <glog/logging.h>
#include <glog/stl_logging.h>

#include "core/plan.h"
#include "duel/screen.h"
#include "util/util.h"

using namespace std;

namespace {

class MutexLock {
public:
  explicit MutexLock(SDL_mutex* mu)
    : mu_(mu) {
    SDL_mutexP(mu_);
  }

  ~MutexLock() {
    SDL_mutexV(mu_);
  }

private:
  SDL_mutex* mu_;
};

void applyCSP(const vector<vector<char> >& csp, Field* f) {
  for (int i = 0; i < 6; i++) {
    if (csp[i].empty())
      continue;
    int h = 1;
    while (h <= 13 && f->Get(i+1, h) != EMPTY) {
      h++;
    }
    for (size_t j = 0; j < csp[i].size() && h <= 13; j++) {
      char c = RED + csp[i][j];
      f->Set(i+1, h++, c);
    }
  }
}

class TrackedField {
public:
  explicit TrackedField(const Field& f) {
    for (int x = 0; x < 6; x++) {
      for (int y = 0; y <=14; y++) {
        char c = f.Get(x+1, y+1);
        set(x, y, c);
      }
    }
  }

  void set(int x, int y, char c) {
    f_[x][y].x = x;
    f_[x][y].y = y;
    f_[x][y].c = c;
  }

  void getConnectedPuyo(int x, int y, char c, vector<int>* pos) {
    if (x < 0 || y < 0 || x >= 6 || x >= 12)
      return;
    if (c != f_[x][y].c)
      return;

    f_[x][y].c = 0;
    pos->push_back(y * 16 + x);
    getConnectedPuyo(x+1, y, c, pos);
    getConnectedPuyo(x-1, y, c, pos);
    getConnectedPuyo(x, y+1, c, pos);
    getConnectedPuyo(x, y-1, c, pos);
  }

  int runChainImpl(int num_chains) {
    bool vanished = false;
    vector<int> pos;
    for (int x = 0; x < 6; x++) {
      for (int y = 0; y < 12; y++) {
        char c = f_[x][y].c;
        if (c == EMPTY)
          break;
        if (c < RED)
          continue;

        pos.clear();
        getConnectedPuyo(x, y, c, &pos);
        // Rollback.
        for (size_t i = 0; i < pos.size(); i++) {
          int vx = pos[i] % 16;
          int vy = pos[i] / 16;
          f_[vx][vy].c = c;
        }

        if (pos.size() < 4) {
          continue;
        }

        vanished = true;
        for (size_t i = 0; i < pos.size(); i++) {
          int vx = pos[i] % 16;
          int vy = pos[i] / 16;
          vanishCell(vx, vy, c, num_chains);
          vanishCell(vx+1, vy, OJAMA, num_chains);
          vanishCell(vx-1, vy, OJAMA, num_chains);
          vanishCell(vx, vy+1, OJAMA, num_chains);
          vanishCell(vx, vy-1, OJAMA, num_chains);
        }
      }
    }

    if (!vanished)
      return num_chains - 1;

    for (int x = 0; x < 6; x++) {
      for (int y = 1; y < 13; y++) {
        TrackedCell* cell = &f_[x][y];
        if (cell->c == EMPTY || f_[x][y-1].c != EMPTY)
          continue;
        int dy = y-1;
        while (dy > 0) {
          if (f_[x][dy-1].c != EMPTY)
            break;
          dy--;
        }

        f_[x][dy] = *cell;
        cell->x = cell->y = cell->c = 0;
      }
    }

    return runChainImpl(num_chains+1);
  }

  void vanishCell(int vx, int vy, char c, int num_chains) {
    if (vx < 0 || vy < 0 || vx >= 6 || vy >= 12)
      return;
    TrackedCell* cell = &f_[vx][vy];
    if (cell->c != c)
      return;

    int ox = cell->x;
    int oy = cell->y;
    chain_[ox][oy] = num_chains;
    cell->c = EMPTY;
  }

  int runChain() {
    CLEAR_ARRAY(chain_);
    return runChainImpl(1);
  }

  const int* chain() const { return &chain_[0][0]; }

private:
  struct TrackedCell {
    int x, y;
    char c;
  };

  TrackedCell f_[6][14];
  int chain_[6][14];
};

SDL_Surface* createCommentSurface() {
  SDL_Surface* s =
    SDL_CreateRGBSurface(SDL_SWSURFACE, 400, 600, 24, 0, 0, 0, 0);
  SDL_Color bg = Screen::bg_color();
  SDL_FillRect(s, NULL,
               SDL_MapRGB(s->format, bg.r, bg.g, bg.b));
  return s;
}

void drawText(SDL_Surface* dst, const char* msg, int x, int y) {
  if (!*msg)
    return;
  SDL_Color fg;
  fg.r = fg.g = fg.b = 0;
  //fg.r = fg.g = fg.b = 255;
  SDL_Surface* s =
    TTF_RenderUTF8_Shaded(Screen::font(), msg, fg, Screen::bg_color());
  CHECK(s) << "Failed to render: " << msg;
  SDL_Rect dr;
  dr.x = x;
  dr.y = y;
  if (dst->w < x + s->w) {
    dr.x = dst->w - s->w;
  }
  SDL_BlitSurface(s, NULL, dst, &dr);
  SDL_FreeSurface(s);
}

std::string ssprintf(const char *format, ...) {
  std::string str;
  str.resize(128);
  for (int i = 0; i < 2; i++) {
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(&str[0], str.size(), format, args);
    va_end(args);
    if (ret < 0)
      abort();
    if (static_cast<size_t>(ret) < str.size()) {
      str.resize(ret);
      return str;
    }
    str.resize(ret + 1);
  }
  abort();
}

void constructChainSearchPattern(vector<vector<vector<char> > >* csp_out) {
  vector<vector<vector<char> > > csp;
  csp.push_back(vector<vector<char> >(7));
  for (int t = 0; t < 4; t++) {
    set<vector<vector<char> > > ncsp;
    for (size_t i = 0; i < csp.size(); i++) {
      for (int j = 0; j < 30; j++) {
        char c = j % 5;
        int x = j / 5;
        vector<vector<char> > a = csp[i];
        a[x].push_back(c);
        if (c < 4)
          a[6].push_back(c);
        ncsp.insert(a);
      }
    }

    csp.clear();
    csp.assign(ncsp.begin(), ncsp.end());
    for (set<vector<vector<char> > >::const_iterator iter = ncsp.begin();
         iter != ncsp.end();
         ++iter) {
      if (!(*iter)[6].empty())
        csp_out->push_back(*iter);
    }
  }

  //fprintf(stderr, "%d chain search patterns\n", (int)csp_out->size());
}

}  // anonymous namespace

class Commentator::Chain {
public:
  Chain(const Field& f, const Plan* p)
    : orig_field_(f),
      num_chains_(0),
      score_(p->score),
      num_turns_(p->numTurns() - 1) {
    TrackedField tf(f);

    int total_score = p->score;
    vector<const Plan*> plans;
    while (p) {
      plans.push_back(p);
      p = p->parent;
    }
    reverse(plans.begin(), plans.end());

    int heights[6];
    for (int x = 0; x < 6; x++) {
      heights[x] = 100;
      for (int y = 0; y < 14; y++) {
        if (f.Get(x+1, y+1) == EMPTY) {
          heights[x] = y;
          break;
        }
      }
    }

    for (size_t i = 0; i < plans.size(); i++) {
      Decision d = plans[i]->decision;
      int x1 = d.x - 1;
      int x2 = d.x - 1 + (d.r == 1) - (d.r == 3);
      int y1, y2;

      if (d.r == 2) {
        y2 = heights[x2]++;
        y1 = heights[x1]++;
      } else {
        y1 = heights[x1]++;
        y2 = heights[x2]++;
      }

      CHECK_GE(14, y1);
      CHECK_GE(14, y2);

      char c1 = f.GetNextPuyo(i * 2);
      char c2 = f.GetNextPuyo(i * 2 + 1);

      future_.push_back(FuturePuyo(x1, y1, c1));
      future_.push_back(FuturePuyo(x2, y2, c2));

      tf.set(x1, y1, c1);
      tf.set(x2, y2, c2);
      num_chains_ = tf.runChain();

      if (plans[i]->score > total_score / 2)
        break;
    }

    memcpy(chain_, tf.chain(), sizeof(chain_));
  }

  Chain(const Field& f, const vector<vector<char> >& csp, int score)
    : orig_field_(f),
      num_chains_(0),
      score_(score) {
    for (size_t i = 0; i < csp[6].size(); i++) {
      added_colors_[csp[6][i]]++;
    }
    Field nf(f);
    applyCSP(csp, &nf);
    TrackedField tf(nf);
    num_chains_ = tf.runChain();
    memcpy(chain_, tf.chain(), sizeof(chain_));
  }

  Chain(const Field& f, int score)
    : orig_field_(f),
      num_chains_(0),
      score_(score) {
    TrackedField tf(f);
    num_chains_ = tf.runChain();
    memcpy(chain_, tf.chain(), sizeof(chain_));
  }

  void draw(Screen* scr, Screen::Shape s, int pi, const int* cmap) const {
    for (size_t i = 0; i < future_.size(); i++) {
      FuturePuyo p = future_[i];
      int c = p.c;
      if (cmap[c]) {
        c = cmap[c];
      }
      scr->drawShape(s, pi, p.x, p.y, c);
    }
  }

  void drawTrace(Screen* scr, int pi, const int* cmap) const {
#if 0
    for (int x = 0; x < 6; x++) {
      for (int y = 0; y < 12; y++) {
        fprintf(stderr, " %d", chain_[x][y]);
      }
      fprintf(stderr, "\n");
    }
#endif
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < 12; j++) {
        int n = chain_[i][j];
        if (!n)
          continue;
        int c = orig_field_.Get(i+1, j+1);
        if (c == EMPTY)
          continue;
        if (cmap[c]) {
          c = cmap[c];
        }
        scr->drawNumber(pi, i, j, n, c);
      }
    }
  }

  int num_chains() const { return num_chains_; }
  int score() const { return score_; }
  int num_turns() const { return num_turns_; }

  string added_colors_str(const int* cmap) const {
    static const char* COLOR_NAMES_JP[] = {
      "赤", "青", "黄", "緑", "紫"
    };
    string r;
    for (map<char, int>::const_iterator iter = added_colors_.begin();
         iter != added_colors_.end();
         ++iter) {
      int c = iter->first + RED;
      int n = iter->second;
      if (cmap[c])
        c = cmap[c];
      if (!r.empty())
        r += ' ';
      r += ssprintf("%s%d", COLOR_NAMES_JP[c-RED], n);
    }
    return r;
  }

private:
  struct FuturePuyo {
    FuturePuyo(int x, int y, int c)
      : x(x), y(y), c(c) {
    }
    int x, y, c;
  };

  const Field orig_field_;
  int chain_[6][14];
  vector<FuturePuyo> future_;
  int num_chains_;
  int score_;
  map<char, int> added_colors_;
  int num_turns_;
};

int runCommentator(void* d) {
  ((Commentator*)d)->run();
  return 0;
}

Commentator::Commentator() {
  CLEAR_ARRAY(is_firing_);
  done_ = false;
  CLEAR_ARRAY(color_map_);
  constructChainSearchPattern(&chain_search_patterns_);

  for (int i = 0; i < 2; i++) {
    texts_[i] = createCommentSurface();
  }

  mu_ = SDL_CreateMutex();
  reset();
  th_ = SDL_CreateThread(&runCommentator, this);
}

Commentator::~Commentator() {
  done_ = true;
  SDL_WaitThread(th_, NULL);
  SDL_DestroyMutex(mu_);
  for (int i = 0; i < 2; i++) {
    SDL_FreeSurface(texts_[i]);
  }
}

void Commentator::run() {
  while (!done_) {
    SDL_Delay(16);

    int pi = 0;
    {
      MutexLock lock(mu_);
      if (!need_update_[0]) {
        pi++;
        if (!need_update_[1])
          continue;
      }
      need_update_[pi] = false;
    }

    Uint32 comment_start_ticks = SDL_GetTicks();

    auto_ptr<Field> f;
    {
      MutexLock lock(mu_);
      f.reset(new Field(fields_[pi]));
      f->SetColorSequence(fields_[pi].GetColorSequence());
    }

    vector<string> events;

    Field nf(*f);
    int chains, score, frames;
    Chain* firing_chain = NULL;
    //fprintf(stderr, "before: %d %s\n", pi, nf.GetDebugOutput().c_str());
    nf.Simulate(&chains, &score, &frames);
    if (score) {
      firing_chain = new Chain(*f, score);
      events.push_back(ssprintf("%d手%d連鎖発火%d点",
                                num_kumipuyos_[pi]+1, chains, score));
      if (max_chains_[pi] / 2 < chains) {
        max_chains_[pi] -= chains;
      }
    }
    //fprintf(stderr, "after: %d %s score=%d c=%p\n",
    //        pi, nf.GetDebugOutput().c_str(), score, firing_chain);

    vector<Plan> plans;
    int depth = (f->GetColorSequence().size() - 1) / 2;
    if (depth > 0)
      f->FindAvailablePlans(depth, &plans);

    float best_score = 200;
    Plan* best_plan = NULL;
    int best_tsubushi_score = 400;
    auto_ptr<Chain> best_tsubushi;
    for (size_t i = 0; i < plans.size(); i++) {
      const Plan& p = plans[i];
      if (best_score < p.score / pow(1.1, p.numTurns() - 1)) {
        best_score = p.score / pow(1.1, p.numTurns() - 1);
        best_plan = &plans[i];
      }

      if (p.score >= 450 && p.score < 5000 &&
          (!p.parent || p.parent->score == 0)) {
        Chain* c = new Chain(*f, &plans[i]);
        if (c->num_chains() > 0 && c->num_chains() <= 3) {
          int score = p.score / c->num_chains();
          if (best_tsubushi_score < score) {
            best_tsubushi_score = score;
            best_tsubushi.reset(c);
          }
        }
      }
    }

    Chain* c = NULL;
    if (best_plan) {
      c = new Chain(*f, best_plan);
    }

    auto_ptr<Chain> best_chain;
    // Slow and incorrect.
    // TODO(hamaji): Remove this and constructChainSearchPattern.
#if 0
    for (size_t i = 0; i < chain_search_patterns_.size(); i++) {
      const vector<vector<char> >& csp = chain_search_patterns_[i];
      Field nf(*f);
      applyCSP(csp, &nf);
      int chains, score, frames;
      nf.Simulate(&chains, &score, &frames);
      if (best_score < score / pow(1.4, csp[6].size())) {
        best_score = score / pow(1.4, csp[6].size());
        best_chain.reset(new Chain(*f, csp, score));
      }
    }
#else
    {
      int heights[6];
      CLEAR_ARRAY(heights);
      for (int x = 0; x < 6; x++) {
        int h = 1;
        while (h <= 13 && f->Get(x+1, h) != EMPTY) {
          h++;
        }
        heights[x] = h;
      }
      float best_score = 100;
      vector<vector<char> > csp(6, vector<char>());
      vector<vector<char> > best_csp(6, vector<char>());
      getPotentialMaxChain(*f, 0, 4, &csp, heights, &best_score, &best_csp);
      if (best_score > 100) {
        best_csp.resize(7);
        for (size_t i = 0; i < 6; i++) {
          for (size_t j = 0; j < best_csp[i].size(); j++) {
            char c = best_csp[i][j];
            if (c < 4)
              best_csp[6].push_back(c);
          }
        }

        Field nf(*f);
        applyCSP(best_csp, &nf);
        int chains, score, frames;
        nf.Simulate(&chains, &score, &frames);
        best_chain.reset(new Chain(*f, best_csp, score));
      }
    }
#endif

    if (best_chain.get()) {
      if (max_chains_[pi] < best_chain->num_chains()) {
        max_chains_[pi] = best_chain->num_chains();
        if (max_chains_[pi] > 3) {
          events.push_back(
            ssprintf("%d手%d連鎖%d点",
                     num_kumipuyos_[pi]+1, max_chains_[pi],
                     best_chain->score()));
        }
      }
    }

    {
      MutexLock lock(mu_);
      fireable_max_chain_[pi].reset(c);
      fireable_tsubushi_[pi].reset(best_tsubushi.release());
      potential_max_chain_[pi].reset(best_chain.release());
      if (firing_chain) {
        is_firing_[pi] = true;
        firing_chain_[pi].reset(firing_chain);
      } else {
        is_firing_[pi] = false;
      }

      copy(events.begin(), events.end(), back_inserter(events_[pi]));
      while (events_[pi].size() > 5)
        events_[pi].pop_front();
    }

    SDL_Surface* surf = NULL;
    TTF_Font* font = Screen::font();
    if (font) {
      surf = createCommentSurface();

      int LX = 7 + 256 * pi;
      int LX2 = 20 + 256 * pi;
      int LH = 20;
      drawText(surf, "本線", LX, LH * 2);
      if (potential_max_chain_[pi].get()) {
        drawText(surf,
                 ssprintf("%d連鎖%d個",
                          potential_max_chain_[pi]->num_chains(),
                          potential_max_chain_[pi]->score() / 70).c_str(),
                 LX2, LH * 3);
        drawText(surf,
                 potential_max_chain_[pi]->added_colors_str(
                   color_map_).c_str(),
                 LX2, LH * 4);
      }
      drawText(surf, "発火可能最大連鎖", LX, LH * 6);
      if (fireable_max_chain_[pi].get()) {
        drawText(surf,
                 ssprintf("%d連鎖%d個%d手",
                          fireable_max_chain_[pi]->num_chains(),
                          fireable_max_chain_[pi]->score() / 70,
                          fireable_max_chain_[pi]->num_turns()).c_str(),
                 LX2, LH * 7);
      }
      drawText(surf, "発火可能潰し", LX, LH * 9);
      if (fireable_tsubushi_[pi].get()) {
        drawText(surf,
                 ssprintf("%d連鎖%d個%d手",
                          fireable_tsubushi_[pi]->num_chains(),
                          fireable_tsubushi_[pi]->score() / 70,
                          fireable_tsubushi_[pi]->num_turns()).c_str(),
                 LX2, LH * 10);
      }
      drawText(surf, "発火中/最終発火", LX, LH * 12);
      if (firing_chain_[pi].get()) {
        drawText(surf,
                 ssprintf("%d連鎖%d個",
                          firing_chain_[pi]->num_chains(),
                          firing_chain_[pi]->score() / 70).c_str(),
                 LX2, LH * 13);
      }
      drawText(surf, "積み込み速度", LX, LH * 15);
      Uint32 elapsed = SDL_GetTicks() - start_ticks_;
      if (num_kumipuyos_[pi] > 0 && num_frames_ > 0 && elapsed > 0) {
        drawText(surf,
                 ssprintf("%.2fPPS  %dFPP",
                          (float)num_kumipuyos_[pi] * 1000 / elapsed,
                          num_frames_ / num_kumipuyos_[pi]).c_str(),
                 LX2, LH * 16);
      }
      if (!ai_msg_[pi].empty()) {
        drawText(surf, ("AI: " + ai_msg_[pi]).c_str(), LX, LH * 22);
      }

      int y = 25;
      for (deque<string>::const_iterator iter = events_[pi].begin();
           iter != events_[pi].end();
           ++iter) {
        drawText(surf, iter->c_str(), LX, LH * y);
        y++;
      }
    }

    {
      MutexLock lock(mu_);
      if (font) {
        swap(texts_[pi], surf);
      }
    }

    if (font) {
      SDL_FreeSurface(surf);
    }

    Uint32 elapsed = SDL_GetTicks() - comment_start_ticks;
    LOG(INFO) << "Comment took " << elapsed << "ms for " << pi;
  }
}

void Commentator::setField(int pi, const Field& f, bool grounded) {
  MutexLock lock(mu_);
  fields_[pi] = f;
  fields_[pi].SetColorSequence(f.GetColorSequence());
  need_update_[pi] = true;
  if (grounded) {
    if (++num_kumipuyos_[pi] == 0) {
      start_ticks_ = SDL_GetTicks();
      num_frames_ = 0;
    }
  }
  //fprintf(stderr, "setField: %d\n", pi);
}

void Commentator::draw(Screen* scr) const {
  assert(scr);
  MutexLock lock(mu_);

  scr->lock();
  for (int pi = 0; pi < 2; pi++) {
    if (Chain* c = fireable_max_chain_[pi].get()) {
      c->draw(scr, Screen::SHAPE_DIAMOND, pi, color_map_);
    }
    if (Chain* c = fireable_tsubushi_[pi].get()) {
      c->draw(scr, Screen::SHAPE_TRIANGLE, pi, color_map_);
    }
  }
  scr->unlock();

  for (int pi = 0; pi < 2; pi++) {
    SDL_Rect dr;
    dr.x = pi * 400;
    dr.y = 0;
    SDL_BlitSurface(texts_[pi], NULL, scr->scr(), &dr);
  }
}

void Commentator::drawMainChain(Screen* scr) const {
  assert(scr);
  MutexLock lock(mu_);

  for (int pi = 0; pi < 2; pi++) {
    if (Chain* c = is_firing_[pi] ?
        firing_chain_[pi].get() : potential_max_chain_[pi].get()) {
      c->drawTrace(scr, pi, color_map_);
    }
  }
}

void Commentator::reset() {
  MutexLock lock(mu_);
  need_update_[0] = need_update_[1] = false;
  num_kumipuyos_[0] = num_kumipuyos_[1] = -1;
  num_frames_ = 0;
  max_chains_[0] = max_chains_[1] = 0;
  for (int i = 0; i < 2; i++) {
    fireable_max_chain_[i].reset();
    fireable_tsubushi_[i].reset();
    potential_max_chain_[i].reset();
  }
}

void Commentator::setColorMap(int oc, int c) {
  color_map_[c] = oc;
}

void Commentator::tick() {
  MutexLock lock(mu_);
  num_frames_++;
}

void Commentator::setAIMessage(int pi, const string& msg) {
  MutexLock lock(mu_);
  ai_msg_[pi] = msg;
}

void Commentator::getPotentialMaxChain(const Field& orig_field,
                                       int d,
                                       int depth,
                                       vector<vector<char> >* csp,
                                       int* heights,
                                       float* best_score,
                                       vector<vector<char> >* best_csp) {
  auto_ptr<Field> f(new Field(orig_field));
  for (int i = 0; i < 30; i++) {
    // 5 because we considers arbitrary puyo.
    char c = i % 5;
    int x = i / 5;

    int h = heights[x];
    if (h > 13)
      continue;

    f->Set(x + 1, h, RED + c);
    int chains, s, frames;
    f->Simulate(&chains, &s, &frames);
    (*csp)[x].push_back(c);
    if (chains) {
      float score = s / pow(1.4, d);
      if (*best_score < score) {
        *best_score = score;
        *best_csp = *csp;
      }
      f.reset(new Field(orig_field));
    } else if (d + 1 < depth) {
      heights[x]++;
      getPotentialMaxChain(*f, d + 1, depth, csp, heights,
                           best_score, best_csp);
      heights[x]--;
      f->Set(x + 1, h, EMPTY);
    }
    (*csp)[x].pop_back();
  }
}
