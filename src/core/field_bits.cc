#include "core/field_bits.h"

#include <sstream>

using namespace std;

FieldBits FieldBits::vanishingSeed() const
{
    //  x
    // xox              -- o is 3-connected
    //
    // xoox  ox   x oo
    //      xo  xoo oo  -- o is 2-connected.
    //
    // So, one 3-connected piece or two 2-connected pieces are necessary and sufficient.

    __m128i u = _mm_and_si128(_mm_slli_epi16(m_, 1), m_);
    __m128i d = _mm_and_si128(_mm_srli_epi16(m_, 1), m_);
    __m128i l = _mm_and_si128(_mm_slli_si128(m_, 2), m_);
    __m128i r = _mm_and_si128(_mm_srli_si128(m_, 2), m_);

    __m128i ud_and = _mm_and_si128(u, d);
    __m128i lr_and = _mm_and_si128(l, r);
    __m128i ud_or = _mm_or_si128(u, d);
    __m128i lr_or = _mm_or_si128(l, r);

    __m128i threes = _mm_or_si128(
        _mm_and_si128(ud_and, lr_or),
        _mm_and_si128(lr_and, ud_or));

    __m128i twos = _mm_or_si128(_mm_or_si128(
        _mm_and_si128(ud_or, lr_or), ud_and), lr_and);

    __m128i two_u = _mm_and_si128(_mm_slli_epi16(twos, 1), twos);
    __m128i two_l = _mm_and_si128(_mm_slli_si128(twos, 2), twos);
    __m128i two_twos = _mm_or_si128(two_u, two_l);

    return FieldBits(_mm_or_si128(threes, two_twos));
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
