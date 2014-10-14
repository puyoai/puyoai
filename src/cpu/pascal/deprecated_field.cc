#include "deprecated_field.h"

#include <cstdlib>
#include <glog/logging.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>

#include "core/decision.h"
#include "core/puyo_controller.h"

using namespace std;

void FieldWithColorSequence::SetColorSequence(const string& sequence) {
  color_sequence_ = sequence;
  for (int i = 0; i < int(color_sequence_.size()); i++) {
    color_sequence_[i] -= '0';
  }
  next_puyo_ = 0;
}

std::string FieldWithColorSequence::GetColorSequence() const {
  string sequence = color_sequence_;
  for (int i = 0; i < int(sequence.size()); i++) {
    sequence[i] += '0';
  }
  return sequence;
}

char FieldWithColorSequence::GetNextPuyo(int n) const {
  assert(!color_sequence_.empty());
  int len = color_sequence_.length();
  if (len == 0) {
    LOG(FATAL) << "You must call DeprecatedField::SetColorSequence() before calling"
               << "DeprecatedField::FindAvailablePlansInternal()";
  }
  return color_sequence_[(next_puyo_ + n) % len];
}

string FieldWithColorSequence::GetDebugOutput() const {
  std::ostringstream s;

  s << "YOKOKU=";
  for (int i = 0; i < (int)color_sequence_.size(); i++) {
    s << (char)('0' + color_sequence_[i]);
  }
  s << std::endl;

  return DeprecatedField::GetDebugOutput() + s.str();
}
