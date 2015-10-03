#include "core/field_bits.h"

#include <sstream>

#include "core/plain_field.h"

using namespace std;

const FieldBits FieldBits::FIELD_MASK_13 = _mm_set_epi16(0, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0);
const FieldBits FieldBits::FIELD_MASK_12 = _mm_set_epi16(0, 0x1FFE, 0x1FFE, 0x1FFE, 0x1FFE, 0x1FFE, 0x1FFE, 0);

FieldBits::FieldBits(const PlainField& pf, PuyoColor c)
{
    __m128i mask = _mm_set1_epi8(static_cast<char>(c));

    // TODO(mayah): should we use _mm_set_epi16? Which is faster?

    union {
        __m128i m;
        std::int16_t s[8];
    } xmm;

    xmm.s[0] = 0;
    for (int i = 1; i <= 6; ++i) {
        __m128i x = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(i)));
        xmm.s[i] = _mm_movemask_epi8(_mm_cmpeq_epi8(x, mask));
    }
    xmm.s[7] = 0;

    m_ = xmm.m;
}

FieldBits FieldBits::mirror() const
{
    // TODO(mayah): Use shuffle for better performance.

    union {
        std::uint16_t xs[8];
        __m128i m;
    };
    m = m_;

    for (int i = 0; i < 4; ++i)
        std::swap(xs[i], xs[7 - i]);

    return m;
}

FieldBits::FieldBits(const std::string& str, char c) :
    FieldBits()
{
    int counter = 0;
    for (int i = str.length() - 1; i >= 0; --i) {
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;

        if (str[i] == c)
            set(x, y);
        counter++;
    }
}

std::string FieldBits::toString() const
{
    stringstream ss;
    for (int y = 15; y >= 0; --y) {
        for (int x = 0; x < 8; ++x) {
            ss << (get(x, y) ? '1' : '0');
        }
        ss << endl;
    }

    return ss.str();
}
