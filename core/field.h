#ifndef CORE_FIELD_H_
#define CORE_FIELD_H_

#include <string>
#include <vector>

#include "core/constant.h"
#include "core/decision.h"

class Plan;

enum Colors {
  EMPTY = 0,
  OJAMA = 1,
  WALL = 2,
  RED = 4,
  BLUE = 5,
  YELLOW = 6,
  GREEN = 7,

  // If this flag is turned on, we don't need to check the cell for vanishment
  // anymore.
  MASK_CHECKED = 0x80,
};

class Field {
 public:
  static const int WIDTH = 6;
  static const int HEIGHT = 12;
  static const int MAP_WIDTH = 1 + WIDTH + 1;
  static const int MAP_HEIGHT = 1 + HEIGHT + 3;
  static const int ERASE_NUM = 4;
  static const int COLORS = 8;

  Field();
  Field(const std::string& url);
  Field(const Field& f);

  // Crears every data this class has.
  void Init();


  // Sets Haipuyo.
  void SetColorSequence(const std::string& sequence);

  // Gets Haipuyo.
  std::string GetColorSequence() const;

  // Put a puyo at a specified position.
  void Set(int x, int y, char color);

  // Get a color of puyo at a specified position.
  char Get(int x, int y) const;

  // Vanish puyos, and adds score. The argument "chains" is used to calculate
  // score.
  bool Vanish(int chains, int* score);

  // After vanishing, drop puyos. You should not Set puyos between vanish and
  // drop.
  void Drop();

  // Simulate chains until the end, and returns chains, score, and frames before
  // finishing the chain.
  void Simulate();
  void Simulate(int* chains, int* score, int* frames);

  // Normal print for debugging purpose.
  std::string GetDebugOutput() const;

  char GetNextPuyo(int n) const;

  /**
   * PLEASE DO NOT USE THE FOLLOWING PUBLIC FUNCTIONS!
   * They are deprecated, and will be removed at any time.
   */
  // depth = 1 -- think about the next pair of puyos.
  // depth = 2 -- think about the next 2 pairs of puyos.
  // depth = 3 -- think about the next 3 pairs of puyos.
  void FindAvailablePlans(int depth, std::vector<Plan>* plans) const;
  // == FindAvailablePlans(3, plans);
  void FindAvailablePlans(std::vector<Plan>* plans) const;

 protected:
  // Clean internal states, related to Vanish and Drop.
  void Clean();

  // Puyo at field[x][y] will not fall or will not be vanished iff
  // y>min_heights[x].
  //
  // After Vanish(): Lowest position a puyo vanished.
  // After Drop(): Lowest position where we should start vanishment-check.
  int min_heights[MAP_WIDTH];
  int next_puyo_;

 private:
  void FillFieldInfo(std::stringstream& ss) const;
  void FindAvailablePlansInternal(const Field& field, const Plan* parent,
                                  int depth, int max_depth,
                                  std::vector<Plan>* plans) const;
  void Drop(int* frames);

  unsigned char field_[MAP_WIDTH][MAP_HEIGHT];
  bool erased_;
  std::string color_sequence_;
};

#endif  // CORE_FIELD_H_
