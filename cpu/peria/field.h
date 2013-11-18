#ifndef CPU_PERIA_FIELD_H_
#define CPU_PERIA_FIELD_H_

#include <string>
#include <vector>

#include "base.h"
#include "decision.h"

enum Colors {
  kEmpty = 0,
  kOjama = 1,
  kWall = 2,
  kRed = 4,
  kBlue = 5,
  kYellow = 6,
  kGreen = 7,

  kMaskChecked = 0x80,
};

class Field {
 public:
  static const int kWidth = 6;
  static const int kHeight = 12;
  static const int kMapWidth = 1 + kWidth + 1;
  static const int kMapHeight = 1 + kHeight + 3;
  static const int kEraseNum = 4;
  static const int kColors = 8;

  Field();
  Field(const string& url);
  Field(const Field& f);

  // Set field area only.
  void SetField(const string& field);

  // Crears every data this class has.
  void Init();

  // Sets Haipuyo.
  void SetColorSequence(const string& sequence);

  // Gets Haipuyo.
  string GetColorSequence() const;

  // Put a puyo at a specified position.
  void Set(int x, int y, char color);

  // Put a pair of puyos.
  void Put(int x, int y, int r);

  // Get a color of puyo at a specified position.
  char Get(int x, int y) const;
  bool IsEmpty(int x, int y) const;

  // Vanish puyos, and adds score. The argument "chains" is used to calculate
  // score.
  bool Vanish(int chains, int* score);

  // Returns true if puyo on (x, y) vanishes.
  bool Vanishable(int x, int y);

  // After vanishing, drop puyos. You should not Set puyos between vanish and
  // drop.
  void Drop();

  // Simulate chains until the end, and returns chains, score, and frames
  // before finishing the chain.
  void Simulate();
  void Simulate(int* chains, int* score, int* frames);

  // Normal print for debugging purpose.
  string GetDebugOutput() const;

  // Returns n-th puyo in the queue.
  char GetNextPuyo(int n) const;

  bool zenkeshi();

  // Returns true if |field| is same
  bool EqualTo(const Field& field, bool visible = true) const;

 protected:
  // Clean internal states, related to Vanish and Drop.
  void Clean_();

  // Puyo at field[x][y] will not fall or will not be vanished iff
  // y>min_heights[x].
  //
  // After Vanish(): Lowest position a puyo vanished.
  // After Drop(): Lowest position where we should start vanishment-check.
  int min_heights[kMapWidth];
  int next_puyo_;

 private:
  void FillFieldInfo(stringstream& ss) const;

  void Drop(int* frames);

  unsigned char field_[kMapWidth][kMapHeight];
  bool erased_;
  bool zenkeshi_;

  string color_sequence_;
};

#endif  // CPU_PERIA_FIELD_H_
