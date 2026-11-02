#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <stddef.h>
#include "../common/geometry.hpp"

namespace tarasenko {
  Ellipse* allocateSharedCircles(const char* name, size_t count);
  void freeSharedCircles(const char* name, Ellipse* ellipses, size_t count);
}

#endif
