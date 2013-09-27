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

  // Put a puyo at a specified position.
  void Set(int x, int y, char color);

  // Get a color of puyo at a specified position.
  // TODO: Returning char seems weird. PuyoColor should be returned instead.
  char Get(int x, int y) const;

  // Simulate chains until the end, and returns chains, score, and frames before
  // finishing the chain.
  void Simulate();
  void Simulate(int* chains, int* score, int* frames);

  // When some puyos are located in the air, drop them.
  void ForceDrop() { Drop(); } 

  // Normal print for debugging purpose.
  std::string GetDebugOutput() const;

 protected:
  // Crears every data this class has.
  void Init();

  // Clean internal states, related to Vanish and Drop.
  void Clean();

  // Vanish puyos, and adds score. The argument "chains" is used to calculate
  // score.
  bool Vanish(int chains, int* score);

  // After vanishing, drop puyos. You should not Set puyos between vanish and
  // drop.
  void Drop();

  // Puyo at field[x][y] will not fall or will not be vanished iff
  // y>min_heights[x].
  //
  // After Vanish(): Lowest position a puyo vanished.
  // After Drop(): Lowest position where we should start vanishment-check.
  int min_heights[MAP_WIDTH];

 private:
  void FillFieldInfo(std::stringstream& ss) const;
  void Drop(int* frames);

  bool erased_;
  friend class FieldRealtime;
};

class FieldWithColorSequence : public Field {
public:
  FieldWithColorSequence() : Field(), next_puyo_(0) {}
  FieldWithColorSequence(const std::string& s) : Field(s), next_puyo_(0) {}
  FieldWithColorSequence(const FieldWithColorSequence& f) :
      Field(f),
      next_puyo_(0) {
    color_sequence_ = f.color_sequence_;
  }

  // Sets Haipuyo.
  void SetColorSequence(const std::string& sequence);
  // Gets Haipuyo.
  std::string GetColorSequence() const;
  // TODO: It's weird to return char.
  char GetNextPuyo(int n) const;

  // Normal print for debugging purpose.
  std::string GetDebugOutput() const;

private:
  std::string color_sequence_;
  int next_puyo_;

  friend class FieldRealtime;
};

#endif  // CORE_FIELD_H_
