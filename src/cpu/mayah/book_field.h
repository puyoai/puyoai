#ifndef CPU_MAYAH_BOOK_FIELD_H_
#define CPU_MAYAH_BOOK_FIELD_H_

#include <string>
#include <vector>

#include "core/plain_field.h"

class BookField {
public:
    BookField(const std::string& name, const std::vector<std::string>& field);

    bool matches(const PlainField&) const;

private:
    std::string name_;
    char field_[PlainField::MAP_WIDTH][PlainField::MAP_HEIGHT];
};

#endif
