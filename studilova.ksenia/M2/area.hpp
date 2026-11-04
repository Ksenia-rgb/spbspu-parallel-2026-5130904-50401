#ifndef AREA_HPP
#define AREA_HPP

#include <cstddef>
#include <vector>
#include "geometry.hpp"

namespace studilova
{
  double calculateArea(const std::vector< circle_t >& circles, size_t threads, size_t tries, unsigned int seed);
}

#endif
