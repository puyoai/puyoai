#ifndef BASE_BUILD_CONFIG_H_
#define BASE_BUILD_CONFIG_H_

// build_config.h defines macro about the platform we're currnetly building on.
//
// Operating system:
//   OS_WIN / OS_MACOSX / OS_LINUX / OS_FREEBSD
//   OS_POSIX = MACOSX or LINUX or OS_FREEBSD
// Compiler:
//   COMPILER_MSVC / COMPILER_GCC / COMPILER_CLANG
//   COMPILER_GCC_COMPATIBLE = COMPILER_GCC or COMPILER_CLANG

#if defined(__linux__)
#  define OS_LINUX 1
#elif defined(__APPLE__)
#  define OS_MACOSX 1
#elif defined(_WIN32)
#  define OS_WIN 1
#elif defined(__FreeBSD__)
#  define OS_FREEBSD 1
#else
#  error "Please add support for your platform base/build_config.h"
#endif

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_FREEBSD)
#  define OS_POSIX 1
#endif

#if defined(__clang__)
#  define COMPILER_CLANG 1
#elif defined(__GNUC__)
#  define COMPILER_GCC 1
#elif defined(_MSC_VER)
#  define COMPILER_MSVC 1
#else
#  error "Please add support for your compiler base/build_config.h"
#endif

// TODO(mayah): What happens when clang-cl is used?
#if defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#  define COMPILER_GCC_COMPATIBLE 1
#endif

#endif // BASE_BUILD_CONFIG_H_
