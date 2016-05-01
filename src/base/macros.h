#ifndef BASE_MACROS_H_
#define BASE_MACROS_H_

// Do not include this file directly.  Use base/base.h instead.

#include <cstddef>

#define UNUSED_VARIABLE(x) (void)(x)

// C++11 allows us to implement code to take the array size.
// If we pass non-array to this function, compile error will happen.
template<typename T, std::size_t size>
constexpr std::size_t ARRAY_SIZE(const T (&)[size]) { return size; }

#define DISALLOW_COPY_AND_ASSIGN(TypeName)     \
    TypeName(const TypeName&) = delete;        \
    void operator=(const TypeName&) = delete

#endif // BASE_MACROS_H_
