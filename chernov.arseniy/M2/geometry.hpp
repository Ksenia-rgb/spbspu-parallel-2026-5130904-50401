#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <string>
#include <unordered_map>
#include <vector>

namespace chernov {
  struct point_t {
    double x, y;
  };

  struct frame_t {
    point_t left_bottom;
    point_t right_top;
  };

  struct ellipse_t {
    point_t center;
    double horizontal_radius;
    double vertical_radius;
  };

  using ellipses_t = std::unordered_map< std::string, ellipse_t >;
  using set_t = std::vector< std::string >;
  using sets_t = std::unordered_map< std::string, set_t >;
}

#endif
