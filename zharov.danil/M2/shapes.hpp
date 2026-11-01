#ifndef SHAPES_HPP
#define SHAPES_HPP
#include <vector>

namespace zharov
{
  struct Point
  {
    long long x, y;
  };

  struct Circle
  {
    long long r;
    Point center;
  };

  struct Frame
  {
    Point left_lower;
    Point right_upper;
  };

  Frame getFrame(const Circle& shape);
  Frame getFrame(const std::vector< Circle >& shapes);
  bool isInside(double x, double y, const Circle& shape);
}

#endif
