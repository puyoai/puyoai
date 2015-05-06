#ifndef CORE_FIELD_BITS_H_
#define CORE_FIELD_BITS_H_

#include <cstdint>

#include <glog/logging.h>
#include <emmintrin.h>

#include "core/field_constant.h"

// FieldBits is a bitset whose size is the same as field.
class FieldBits {
public:
    FieldBits() : c_{} {}

    __m128i& xmm() { return m_; }

private:
    union {
        std::uint16_t c_[8];
        __m128i m_;
    };
};

#endif // CORE_FIELD_BITS_H_
