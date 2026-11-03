#ifndef STUDILOVA_AREA_HPP
#define STUDILOVA_AREA_HPP

#include <cstddef>
#include <vector>
#include "geometry.hpp"

namespace studilova
{
  double calculateArea(const std::vector< circle_t >& circles, size_t tries, unsigned int seed);
}

#endif
