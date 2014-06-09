#ifndef DUEL_GUI_H_
#define DUEL_GUI_H_

#include <string>
#include "core/key.h"

class FieldRealtime;

class Gui {
 public:
  static Gui* Create();

  Gui() {}
  virtual ~Gui() {}

  virtual void Draw(const FieldRealtime&, const std::string&) {}

  virtual void Flip() {}

  virtual Key GetKey() { return KEY_NONE; }
};

#endif  // DUEL_GUI_H_
