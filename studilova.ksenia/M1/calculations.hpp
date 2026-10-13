#ifndef CALCULATIONS_HPP
#define CALCULATIONS_HPP

#include <vector>

namespace studilova
{
  struct Circle
  {
    double radius;
    double x;
    double y;
  };

  struct Rectangle
  {
    double minX;
    double minY;
    double maxX;
    double maxY;
  };

  bool containsPoint(const Circle& circle, double x, double y);

  Rectangle findRectangle(const std::vector< Circle >& circles);
}

#endif
