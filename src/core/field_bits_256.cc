#ifdef __AVX2__

// This file does compile if -mavx2 is specified or -mnative is specified and CPU has AVX2.

#include "core/field_bits_256.h"

using namespace std;

string FieldBits256::toString() const
{
    stringstream ss;
    for (int y = 15; y >= 0; --y) {
        for (int x = 0; x < 8; ++x) {
            ss << (get(HighLow::HIGH, x, y) ? '1' : '0');
        }
        ss << "   ";
        for (int x = 0; x < 8; ++x) {
            ss << (get(HighLow::LOW, x, y) ? '1' : '0');
        }
        ss << endl;
    }

    return ss.str();
}

FieldBits256 FieldBits256::vanishingSeed() const
{
    // See FieldBits::vanishingSeed for the implementation details.

    __m256i u = _mm256_and_si256(_mm256_slli_epi16(m_, 1), m_);
    __m256i d = _mm256_and_si256(_mm256_srli_epi16(m_, 1), m_);
    __m256i l = _mm256_and_si256(_mm256_slli_si256(m_, 2), m_);
    __m256i r = _mm256_and_si256(_mm256_srli_si256(m_, 2), m_);

    __m256i ud_and = _mm256_and_si256(u, d);
    __m256i lr_and = _mm256_and_si256(l, r);
    __m256i ud_or = _mm256_or_si256(u, d);
    __m256i lr_or = _mm256_or_si256(l, r);

    __m256i threes = _mm256_or_si256(
        _mm256_and_si256(ud_and, lr_or),
        _mm256_and_si256(lr_and, ud_or));

    __m256i twos = _mm256_or_si256(_mm256_or_si256(
        _mm256_and_si256(ud_or, lr_or), ud_and), lr_and);

    __m256i two_u = _mm256_and_si256(_mm256_slli_epi16(twos, 1), twos);
    __m256i two_l = _mm256_and_si256(_mm256_slli_si256(twos, 2), twos);
    __m256i two_twos = _mm256_or_si256(two_u, two_l);

    return _mm256_or_si256(threes, two_twos);
}

#endif // __AVX2__
