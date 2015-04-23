#ifndef HAMAJI_FIELD_H_
#define HAMAJI_FIELD_H_

#include <string>
#include <vector>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo_seq.h"
#include "core/puyo_color.h"

#include "base.h"

typedef unsigned char byte;

class LP;

struct Chain {
  int chains;
  int score;
  int frames;
  int depth;
  int vanished;

  struct ByScore {
    bool operator()(const Chain* a, const Chain* b) const {
      return a->score > b->score;
    }
  };

  struct ByBest {
    bool operator()(const Chain* a, const Chain* b) const {
      return a->chains * 256 - a->vanished > b->chains * 256 - b->vanished;
    }
  };
};

class LF : public CoreField {
 public:
  LF() {}
  explicit LF(const CoreField& cf) : CoreField(cf) {}
  explicit LF(const string& url) : CoreField(url) {}

  // Put a puyo at a specified position.
  void Set(int x, int y, PuyoColor c) { setPuyoAndHeight(x, y, c); }

  // Get a color of puyo at a specified position.
  PuyoColor Get(int x, int y) const { return color(x, y); }

  // Simulate chains until the end, and returns chains, score, and frames before
  // finishing the chain.
  void Simulate();
  void Simulate(int* chains, int* score, int* frames);
  void SimulateFromChain(int init_chains,
                         int* chains, int* score, int* frames);

  // Normal print for debugging purpose.
  const string GetDebugOutput() const;

  // TODO(mayah): Consider using Plan::iterateAvailablePlans().
  // depth = 1 -- think about the next pair of puyos.
  // depth = 2 -- think about the next 2 pairs of puyos.
  // depth = 3 -- think about the next 3 pairs of puyos.
  void FindAvailablePlans(const KumipuyoSeq& next, int depth, vector<LP>* plans);
  // == FindAvailablePlans(3, plans);
  void FindAvailablePlans(const KumipuyoSeq& next, vector<LP>* plans);

  // Slow, maybe.
  int PutDecision(Decision decision, PuyoColor c1, PuyoColor c2,
                  int* chigiri_frames = NULL);

  string query_string() const;
  string url() const { return "http://www.inosendo.com/puyo/rensim/?" + query_string(); }
  int countPuyo() const { return countPuyos(); }
  int countColorPuyo() const;
  bool hasPuyo() const;

  int getBestChainCount(int* ignition_puyo_cnt = NULL,
                        int* useful_chain_cnt_out = NULL,
                        int* vanished_puyo_cnt = NULL) const;

  int getOjamaFilmHeight(int* hidden_color_puyo_cnt = NULL) const;

  void getProspectiveChains(vector<Chain*>* pchains,
                            int i, int d, int depth) const;
  void getProspectiveChains(vector<Chain*>* pchains) const;

  static const string parseNext(const char* p);
  static void parseNext(const char* p, string* next);

  bool isEqual(const LF& f) const;

  bool complementOjamasDropped(const LF& f);

 private:
  void FindAvailablePlansInternal(const LF& field, const KumipuyoSeq& next,
                                  const LP* parent,
                                  int depth, int max_depth,
                                  vector<LP>* plans);
};

class LP {
 public:
  Decision getFirstDecision() const;

  // Previous state.
  const LP* parent;
  // Decision made for this plan.
  Decision decision;
  // Future field state (apply the state to the previous state).
  LF field;
  // Score we get with the future field.
  int score;
  int chain_cnt;
  int chigiri_frames;

  vector<double> evals;
};

#endif  // HAMAJI_FIELD_H_
