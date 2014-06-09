#ifndef CORE_FIELD_H_
#define CORE_FIELD_H_

#include <string>
#include <vector>

#include "core/constant.h"
#include "core/puyo.h"

// DEPRECATED. A user should use Field instead.
// This class contains unnecessary garbages.
class DeprecatedField {
 public:
  static const int WIDTH = 6;
  static const int HEIGHT = 12;
  static const int MAP_WIDTH = 1 + WIDTH + 1;
  static const int MAP_HEIGHT = 1 + HEIGHT + 3;

  DeprecatedField();
  DeprecatedField(const std::string& url);
  DeprecatedField(const DeprecatedField& f);

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

 private:
  void FillFieldInfo(std::stringstream& ss) const;
  void Drop(int* frames);

  byte field_[MAP_WIDTH][MAP_HEIGHT];

  // Puyo at field[x][y] will not fall or will not be vanished iff
  // y>min_heights[x].
  //
  // After Vanish(): Lowest position a puyo vanished.
  // After Drop(): Lowest position where we should start vanishment-check.
  int min_heights[MAP_WIDTH];

  bool erased_;
  friend class FieldRealtime;
};

// DEPRECATED. A user should mange color sequence by himself.
class FieldWithColorSequence : public DeprecatedField {
public:
  FieldWithColorSequence() : DeprecatedField(), next_puyo_(0) {}
  FieldWithColorSequence(const std::string& s) : DeprecatedField(s), next_puyo_(0) {}
  FieldWithColorSequence(const FieldWithColorSequence& f) :
      DeprecatedField(f),
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
