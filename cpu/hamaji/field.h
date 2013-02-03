#ifndef FIELD_H_
#define FIELD_H_

#include <string>
#include <vector>

#include "../../core/decision.h"
#include "../../core/field.h"

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

class LF {
 public:
  static const int WIDTH = 6;
  static const int HEIGHT = 12;
  static const int MAP_WIDTH = 1 + WIDTH + 1;
  static const int MAP_HEIGHT = 1 + HEIGHT + 3;
  static const int ERASE_NUM = 4;
  static const int COLORS = 8;

  LF();
  explicit LF(const string& url);

  // Crears every data this class has.
  void Init();

  // Put a puyo at a specified position.
  void Set(int x, int y, char color);

  // Get a color of puyo at a specified position.
  byte Get(int x, int y) const;

  // Vanish puyos, and adds score. The argument "chains" is used to calculate
  // score.
  bool Vanish(int chains, int* score);

  // After vanishing, drop puyos. You should not Set puyos between vanish and
  // drop.
  void Drop();

  void SafeDrop();

  // Simulate chains until the end, and returns chains, score, and frames before
  // finishing the chain.
  void Simulate();
  void Simulate(int* chains, int* score, int* frames);
  void SimulateFromChain(int init_chains,
                         int* chains, int* score, int* frames);

  // Normal print for debugging purpose.
  const string GetDebugOutput() const;
  const string GetDebugOutput(const string& next) const;
  static const string GetDebugOutputForNext(const string& next);

  // depth = 1 -- think about the next pair of puyos.
  // depth = 2 -- think about the next 2 pairs of puyos.
  // depth = 3 -- think about the next 3 pairs of puyos.
  void FindAvailablePlans(const string& next, int depth, vector<LP>* plans);
  // == FindAvailablePlans(3, plans);
  void FindAvailablePlans(const string& next, vector<LP>* plans);

  // Slow, maybe.
  int PutDecision(Decision decision, char c1, char c2,
                  int* chigiri_frames = NULL);

  const string query_string() const;
  const string url() const;

  int countPuyo() const;
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

 protected:
  // Clean internal states, related to Vanish and Drop.
  void Clean_();

  byte field[MAP_WIDTH][MAP_HEIGHT];
  // Puyo at field[x][y] will not fall or will not be vanished iff
  // y>min_heights[x].
  //
  // After Vanish(): Lowest position a puyo vanished.
  // After Drop(): Lowest position where we should start vanishment-check.
  byte min_heights[MAP_WIDTH];

 private:
  void FillFieldInfo(stringstream& ss) const;
  void FindAvailablePlansInternal(const LF& field, const string& next,
                                  const LP* parent,
                                  int depth, int max_depth,
                                  vector<LP>* plans);
  void Drop(int* frames);
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

#endif  // FIELD_H_
