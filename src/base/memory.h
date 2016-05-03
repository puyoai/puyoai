#ifndef BASE_MEMORY_H_
#define BASE_MEMORY_H_

#include <cstdlib>
#include <memory>

namespace base {

// Freer calls free()
struct Freer {
    void operator()(void* x) { free(x); }
};

template<typename T>
using unique_ptr_malloc = std::unique_ptr<T, Freer>;

} // base

#endif // BASE_MEMORY_H_
