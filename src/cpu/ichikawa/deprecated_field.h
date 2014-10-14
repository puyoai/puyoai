#ifndef DEPRECATED_FIELD_H_
#define DEPRECATED_FIELD_H_

#include <string>
#include <vector>

#include "core/constant.h"
#include "core/puyo_color.h"
#include "core/field/rensa_result.h"
#include "core/field/core_field.h"

// DEPRECATED. A user should use Field instead.
// This class contains unnecessary garbages.
class DeprecatedField : public FieldConstant {
 public:

  DeprecatedField() {}
  DeprecatedField(const std::string& url) : inner_(url) {}
  DeprecatedField(const DeprecatedField& f) : inner_(f.inner_) {}

  // Put a puyo at a specified position.
  void Set(int x, int y, char c) {
    inner_.setPuyoAndHeight(x, y, toPuyoColor(c));
  }

  // Get a color of puyo at a specified position.
  // TODO: Returning char seems weird. PuyoColor should be returned instead.
  PuyoColor Get(int x, int y) const {
    return inner_.color(x, y);
  }

  // Simulate chains until the end, and returns chains, score, and frames before
  // finishing the chain.
  void Simulate() { inner_.simulate(); }
  void Simulate(int* chains, int* score, int* frames) {
    RensaResult rensaResult = inner_.simulate();
    *chains = rensaResult.chains;
    *score = rensaResult.score;
    *frames = rensaResult.frames;
  }

  // When some puyos are located in the air, drop them.
  void ForceDrop() { inner_.forceDrop(); }

  // Normal print for debugging purpose.
  std::string GetDebugOutput() const { return inner_.toDebugString(); }

  const CoreField& inner() const { return inner_; }

 private:
  CoreField inner_;
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
};

#endif  // DEPRECATED_FIELD_H_
