#ifndef MONTECARLO_HPP
#define MONTECARLO_HPP
#include <cstddef>
#include <vector>
#include "shapes.hpp"

namespace zharov
{
  size_t countInside(const std::vector< Circle >& shapes, Frame frame, size_t tries, size_t seed);
  double calcArea(const std::vector< Circle >& shapes, size_t threads, size_t tries, size_t seed);
}

#endif
