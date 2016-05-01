#ifndef BASE_COMPILER_SPECIFIC_H_
#define BASE_COMPILER_SPECIFIC_H_

// Do not include this file directly.  Use base/base.h instead.

#include "build/build_config.h"

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

// NOINLINE_UNLESS_RELEASE is defined in Release build.
#ifndef NOINLINE_UNLESS_RELEASE
#ifdef OS_WIN
#    define NOINLINE_UNLESS_RELEASE __declspec(noinline)
#else
#    define NOINLINE_UNLESS_RELEASE __attribute__ ((noinline))
#endif
#endif

// CLANG_ALWAYS_INLINE sets __attribute__((always_inline)) only when clang is used.
#ifdef COMPILER_CLANG
#define CLANG_ALWAYS_INLINE __attribute__((always_inline))
#else
#define CLANG_ALWAYS_INLINE
#endif

#endif // BASE_COMPILER_SPECIFIC_H_
