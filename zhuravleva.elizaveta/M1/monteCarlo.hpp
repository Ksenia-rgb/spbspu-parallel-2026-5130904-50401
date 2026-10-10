#ifndef MONTE_CARLO_HPP
#define MONTE_CARLO_HPP

#include <vector>

namespace zhuravleva
{
  struct Circle
  {
    int radius, x, y;
  };

  struct Bounds
  {
    double minX, maxX, minY, maxY;
  };

  bool isInside(const Circle& circle, double x, double y);
  Bounds getBounds(const std::vector<Circle>& circles);
}

#endif
