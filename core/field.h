#ifndef CORE_FIELD_H_
#define CORE_FIELD_H_

#include <string>
#include <vector>

#include "core/basic_field.h"
#include "core/constant.h"
#include "core/puyo.h"

class Plan;

class Field : public BasicField {
 public:
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
  // TODO: Returning char seems weird. PuyoColor should be returned instead.
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
  void Drop(int* frames);

  bool erased_;
  std::string color_sequence_;
};

#endif  // CORE_FIELD_H_
