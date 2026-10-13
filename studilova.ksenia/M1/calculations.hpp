#ifndef CALCULATIONS_HPP
#define CALCULATIONS_HPP

namespace studilova
{
  struct Circle
  {
    double radius;
    double x;
    double y;
  };

  bool containsPoint(const Circle& circle, double x, double y);
}

#endif
