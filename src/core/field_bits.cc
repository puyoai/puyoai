#include "core/field_bits.h"

#include <sstream>

#include "core/plain_field.h"

using namespace std;

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
