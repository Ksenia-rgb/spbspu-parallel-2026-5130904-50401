#ifndef TARASENKO_YAROSLAV_M2_SHARED_MEMORY_HPP
#define TARASENKO_YAROSLAV_M2_SHARED_MEMORY_HPP

#include <stddef.h>
#include "geometry.hpp"

namespace tarasenko {
  Circle* allocateSharedCircles(const char* name, size_t count);
  void freeSharedCircles(const char* name, Circle* circles, size_t count);
}

#endif
