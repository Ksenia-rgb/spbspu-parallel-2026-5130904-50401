#ifndef MONTE_CARLO_HPP
#define MONTE_CARLO_HPP

#include <cstddef>
#include <vector>
#include "shared_circles.hpp"

namespace goltsov
{
  bool isInside(const Circle* s, const double& x, const double& y);

  size_t calc(const std::vector< const Circle* >& data, const size_t& tests, const size_t& seed, const double& min_x,
      const double& min_y, const double& max_x, const double& max_y);

  double area(const std::vector< const Circle* >& data, const size_t& threads, const size_t& tries,
      const size_t& start_seed, const double& min_x, const double& min_y, const double& max_x, const double& max_y);
}

#endif
