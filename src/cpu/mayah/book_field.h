#ifndef CPU_MAYAH_BOOK_FIELD_H_
#define CPU_MAYAH_BOOK_FIELD_H_

#include <string>
#include <vector>

#include "core/plain_field.h"

class BookField {
public:
    BookField(const std::string& name, const std::vector<std::string>& field, bool partial = false);

    bool matches(const PlainField&) const;

    void merge(const BookField&);
    BookField mirror() const;

    bool isPartial() const { return partial_; }
private:
    std::string name_;
    bool partial_;
    char field_[PlainField::MAP_WIDTH][PlainField::MAP_HEIGHT];
};

#endif
