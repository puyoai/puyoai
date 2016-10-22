#pragma once

#include <glog/logging.h>

#include "base/base.h"

namespace peria {

// TODO: Consider moving this to base/macros.h?
#define NOTREACHED() CHECK(false) << "This line should not be reached.";

#define PERIA_ROOT SRC_DIR    "/cpu/peria"
#define BOOK_DIR   PERIA_ROOT "/book"

}  // namespace peria
