#ifndef CPU_MAYAH_BOOK_READER_H_
#define CPU_MAYAH_BOOK_READER_H_

#include <string>

#include "core/plain_field.h"

typedef char Variable;

struct BookHandField {
    std::string name;
    Variable vars[PlainField::MAP_WIDTH][PlainField::MAP_HEIGHT];
};

class BookReader {
public:
    void parse(const std::string& filename);
};

#endif
