#include "area.hpp"

#include <algorithm>
#include <functional>
#include <future>
#include <limits>
#include <random>
#include <vector>

namespace petrov {
  using circles_t = std::vector< petrov::circle_t >;

  struct box_t {
    double left;
    double bottom;
    double right;
    double top;
  };

  struct hits_t {
    std::size_t cover;
    std::size_t comm;
  };

  bool isInside(const petrov::circle_t &circle, double x, double y)
  {
    const double dx = x - circle.x;
    const double dy = y - circle.y;
    return (dx * dx + dy * dy) <= (circle.r * circle.r);
  }
}