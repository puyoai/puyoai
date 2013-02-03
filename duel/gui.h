#ifndef __GUI_H__
#define __GUI_H__

#include <string>

#include "field.h"

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

#endif  // __GUI_H__
