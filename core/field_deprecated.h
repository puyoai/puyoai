#ifndef CORE_FIELD_DEPRECATED_H_
#define CORE_FIELD_DEPRECATED_H_

#include <map>
#include <string>
#include <vector>

#include "core/field.h"

class FieldDeprecated : public Field {
 public:
  FieldDeprecated(const std::string& url) : Field(url) {}

  static void GetPossibleFields(
      const FieldDeprecated& field, char c1, char c2,
      std::vector<std::pair<Decision, FieldDeprecated> >* ret);
};

#endif  // CORE_FIELD_DEPRECATED_H_
