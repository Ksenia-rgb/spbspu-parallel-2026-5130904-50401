#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <vector>

namespace studilova
{
  struct circle_t
  {
    int radius;
    int x;
    int y;
  };

  struct rectangle_t
  {
    double minX;
    double minY;
    double maxX;
    double maxY;
  };

  rectangle_t findFrame(const circle_t& circle);
  rectangle_t findFrame(const std::vector< circle_t >& circles);
}

#endif
