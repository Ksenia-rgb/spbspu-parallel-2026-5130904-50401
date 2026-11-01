#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

namespace chernov {
  struct point_t
  {
    double x, y;
  };

  struct ellipse_t
  {
    point_t center;
    double horizontal_radius;
    double vertical_radius;
  };

  struct frame_t
  {
    point_t left_bottom;
    point_t right_top;
  };
}

#endif
