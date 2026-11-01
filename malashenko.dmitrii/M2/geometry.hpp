#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP
#include <vector>


namespace malashenko
{
  struct Circle
  {
    int radius;
    int x;
    int y;
  };

  struct BoundingBox
  {
    long long min_x;
    long long min_y;
    long long max_x;
    long long max_y;
  };

  BoundingBox getBoundingBox(const Circle& circle);

  BoundingBox getBoundingBox(const std::vector<Circle>& circles);

  bool isInside(const Circle& circle, double x, double y);
}

#endif