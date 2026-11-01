#ifndef TARASENKO_YAROSLAV_M2_GEOMETRY_HPP
#define TARASENKO_YAROSLAV_M2_GEOMETRY_HPP

namespace tarasenko {
  struct RealPoint
  {
    double x;
    double y;
  };

  struct Circle
  {
    double radius;
    RealPoint center;
  };

  struct Rectangle
  {
    RealPoint left_bottom;
    RealPoint right_top;
  };
}

#endif
