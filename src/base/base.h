#ifndef BASE_BASE_H_
#define BASE_BASE_H_

#include "base/compiler_specific.h"
#include "base/macros.h"

#ifdef OS_WIN
using ssize_t = long long;
#endif

#endif  // BASE_BASE_H_
