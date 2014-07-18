#ifndef CORE_FIELD_FIELD_PRETTY_PRINTER_H_
#define CORE_FIELD_FIELD_PRETTY_PRINTER_H_

class PlainField;
class KumipuyoSeq;

class FieldPrettyPrinter {
public:
    static void print(const PlainField&, const KumipuyoSeq&);
    static void printMultipleFields(const PlainField&, const KumipuyoSeq&,
                                    const PlainField&, const KumipuyoSeq&);
};

#endif
