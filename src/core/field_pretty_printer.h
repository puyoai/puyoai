#ifndef CORE_FIELD_FIELD_PRETTY_PRINTER_H_
#define CORE_FIELD_FIELD_PRETTY_PRINTER_H_

#include <initializer_list>
#include <string>

class KumipuyoSeq;
class PlainField;

class FieldPrettyPrinter {
public:
    static std::string toStringFromMultipleFields(std::initializer_list<PlainField>,
                                                  std::initializer_list<KumipuyoSeq>);

    static void print(const PlainField&, const KumipuyoSeq&);
    static void printMultipleFields(std::initializer_list<PlainField>,
                                    std::initializer_list<KumipuyoSeq>);
};

#endif
