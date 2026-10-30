#ifndef WORKER_HPP
#define WORKER_HPP

#include <cstddef>
#include <vector>
#include <utility>
#include "structures.hpp"

namespace vasyakin
{
  bool isInside(double x, double y, const vasyakin::circle_t& circle);
  vasyakin::rectangle_t findRectangle(const std::vector< vasyakin::circle_t >& circles);
  size_t calc(size_t tries, unsigned int seed, const std::vector< vasyakin::circle_t >& circles);
  double area(size_t threads, size_t tries, unsigned int seed, const std::vector< vasyakin::circle_t >& circles,
      long long& time_ms);
  int worker(int cmd_fd, int res_fd, unsigned int seed);
}

#endif
