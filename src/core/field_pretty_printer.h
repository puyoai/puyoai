#ifndef CORE_FIELD_FIELD_PRETTY_PRINTER_H_
#define CORE_FIELD_FIELD_PRETTY_PRINTER_H_

#include <initializer_list>
#include <string>

#include "core/field/core_field.h"

class KumipuyoSeq;

class FieldPrettyPrinter {
public:
    static std::string toStringFromMultipleFields(std::initializer_list<PlainField>,
                                                  std::initializer_list<KumipuyoSeq>);

    static void print(const CoreField&, const KumipuyoSeq&);
    static void printMultipleFields(std::initializer_list<PlainField>,
                                    std::initializer_list<KumipuyoSeq>);
};

#endif
