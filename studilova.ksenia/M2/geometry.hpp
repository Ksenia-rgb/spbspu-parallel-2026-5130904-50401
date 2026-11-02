#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

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
}

#endif
