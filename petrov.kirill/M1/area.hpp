#ifndef AREA_HPP
#define AREA_HPP

#include <cstddef>
#include <vector>

namespace petrov {
  struct circle_t {
    double x;
    double y;
    double radius;
  };

  struct areas_t {
    double covered;
    double common;
  };

  areas_t estimateAreas(
      const std::vector< circle_t > &circles, std::size_t threads, std::size_t tries, std::size_t seed);
}

#endif
