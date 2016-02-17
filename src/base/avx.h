#ifndef BASE_AVX_H_
#define BASE_AVX_H_

#if !defined(_MSC_VER)
#include <x86intrin.h>
#endif

#ifdef __AVX__

namespace avx {

union Decomposer256 {
    __m256i m;
    std::uint64_t ui64[4];
    std::uint32_t ui32[8];
    std::uint16_t ui16[16];
    std::uint8_t ui8[32];
};

}

#endif // __AVX__
#endif // BASE_AVX_H_
