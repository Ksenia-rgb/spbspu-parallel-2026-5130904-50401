#ifndef AREA_HPP
#define AREA_HPP

#include <vector>

namespace petrov {
  struct circle_t {
    double x;
    double y;
    double r;
  };

  struct areas_t {
    double cover;
    double comm;
  };

  areas_t estimateAreas(const std::vector< circle_t > &circles, size_t threads, size_t tries,
    size_t seed);
}

#endif
