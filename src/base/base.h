#ifndef BASE_BASE_H_
#define BASE_BASE_H_

#include <cstddef>
#include "base/compiler_specific.h"

#define UNUSED_VARIABLE(x) (void)(x)

// C++11 allows us to implement code to take the array size.
// If we pass non-array to this function, compile error will happen.
template<typename T, std::size_t size>
constexpr std::size_t ARRAY_SIZE(const T (&)[size]) { return size; }

#endif  // BASE_BASE_H_
