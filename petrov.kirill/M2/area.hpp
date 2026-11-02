#ifndef AREA_HPP
#define AREA_HPP

#include <cstddef>
#include <vector>

#include "shape.hpp"

namespace petrov {
  double countArea(const std::vector< Shape >& shapes, size_t threads, size_t tries,
      size_t seed);
}

#endif
