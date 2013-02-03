#ifndef CAPTURE_COMMENTATOR_H_
#define CAPTURE_COMMENTATOR_H_

#include <memory>
#include <queue>

#include <SDL.h>

#include "core/field.h"

using namespace std;

class Screen;

class Commentator {
public:
  Commentator();
  ~Commentator();

  void run();
  void setField(int pi, const Field& f, bool grounded);
  void setAIMessage(int pi, const string& msg);

  void draw(Screen* scr) const;
  void drawMainChain(Screen* scr) const;

  void reset();

  void setColorMap(int oc, int c);

  void tick();

private:
  class Chain;

  static void getPotentialMaxChain(const Field& orig_field,
                                   int d,
                                   int depth,
                                   vector<vector<char> >* csp,
                                   int* heights,
                                   float* best_score,
                                   vector<vector<char> >* best_csp);

  Field fields_[2];
  string ai_msg_[2];
  SDL_Thread* th_;
  mutable SDL_mutex* mu_;
  volatile bool done_;
  volatile bool need_update_[2];

  auto_ptr<Chain> fireable_max_chain_[2];
  auto_ptr<Chain> fireable_tsubushi_[2];
  auto_ptr<Chain> potential_max_chain_[2];
  auto_ptr<Chain> firing_chain_[2];
  bool is_firing_[2];
  int color_map_[10];
  deque<string> events_[2];
  SDL_Surface* texts_[2];

  vector<vector<vector<char> > > chain_search_patterns_;

  Uint32 start_ticks_;
  int num_kumipuyos_[2];
  int num_frames_;
  int max_chains_[2];
};

#endif  // CAPTURE_COMMENTATOR_H_
