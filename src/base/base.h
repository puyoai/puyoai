#ifndef BASE_BASE_H_
#define BASE_BASE_H_

#include <cstddef>

#define UNUSED_VARIABLE(x) (void)(x)

// C++11 allows us to implement code to take the array size.
// If we pass non-array to this function, compile error will happen.
template<typename T, std::size_t size>
constexpr std::size_t ARRAY_SIZE(const T (&)[size]) { return size; }

#ifndef __has_feature
#  define __has_feature(x) 0
#endif

#ifndef __has_extension
#  define __has_extension(x) 0
#endif

#if __has_extension(attribute_deprecated_with_message)
#  define DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#  define DEPRECATED __attribute__((deprecated()))
#else
#  define DEPRECATED_MSG(msg)
#  define DEPRECATED
#endif

#endif  // BASE_BASE_H_
