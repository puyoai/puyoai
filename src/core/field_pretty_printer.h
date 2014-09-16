#ifndef CORE_FIELD_FIELD_PRETTY_PRINTER_H_
#define CORE_FIELD_FIELD_PRETTY_PRINTER_H_

#include <string>

class PlainField;
class KumipuyoSeq;

class FieldPrettyPrinter {
public:
    static std::string toStringFromMultipleFields(const PlainField&, const KumipuyoSeq&,
                                                  const PlainField&, const KumipuyoSeq&);

    static void print(const PlainField&, const KumipuyoSeq&);
    static void printMultipleFields(const PlainField&, const KumipuyoSeq&,
                                    const PlainField&, const KumipuyoSeq&);
};

#endif
