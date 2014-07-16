#ifndef CPU_MAYAH_BOOK_READER_H_
#define CPU_MAYAH_BOOK_READER_H_

#include <string>
#include <vector>

#include "core/plain_field.h"
#include "book_field.h"

class BookReader {
public:
    static std::vector<BookField> parse(const std::string& filename);
};

#endif
