#ifndef BASE_PORT_H_
#define BASE_PORT_H_

// Do not include this file directly.  Use base/base.h instead.

#include <cstdint>

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

#if OS_WIN
using ssize_t = int64;
#endif

#endif  // BASE_PORT_H_
