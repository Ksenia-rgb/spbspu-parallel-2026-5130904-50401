#include <cstddef>
#include <vector>
#include <utility>
#include "structures.hpp"

namespace vasyakin
{
  bool isInside(double x, double y, const vasyakin::circle_t& circle);
  vasyakin::rectangle_t findRectangle(const std::vector< vasyakin::circle_t >& circles);
  std::pair< size_t, size_t > calc(size_t tries, unsigned int seed, const std::vector< vasyakin::circle_t >& circles);
  std::pair< double, double > area(size_t threads, size_t tries, unsigned int seed,
      const std::vector< vasyakin::circle_t >& circles, long long& time_ms);
  int worker(int cmd_fd, int res_fd, unsigned int seed);
}
