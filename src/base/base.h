#ifndef BASE_BASE_H_
#define BASE_BASE_H_

#define CLEAR_ARRAY(a) memset(a, 0, sizeof(a))

typedef unsigned char byte;
#define UNUSED_VARIABLE(x) (void)(x)

#ifndef __has_feature
#  define __has_feature(x) 0
#endif

#ifndef __has_extension
#  define __has_extension(x) 0
#endif

#if __has_feature(cxx_override_control)
#  define OVERRIDE override
#else
#  define OVERRIDE
#endif

#if __has_extension(attribute_deprecated_with_message)
#  define DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#  define DEPRECATED __attribute__((deprecated()))
#else
#  define DEPRECATED_MSG(msg)
#  define DEPRECATED
#endif

struct noncopyable {
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

#endif  // BASE_BASE_H_
