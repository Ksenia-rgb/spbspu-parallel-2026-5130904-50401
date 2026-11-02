#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include "point.hpp"

namespace donkeev
{
  struct rectangle_t
  {
    point_t bottom_left_;
    point_t top_right_;
  };
}

#endif
