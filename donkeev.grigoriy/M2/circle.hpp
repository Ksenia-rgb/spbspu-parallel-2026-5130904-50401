#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include "point.hpp"
#include "rectangle.hpp"

namespace donkeev
{
  struct circle_t
  {
    circle_t(int, int, int);

    int getRadius() const;
    point_t getCenter() const;

    rectangle_t getBoundingBox() const;
  private:
    int radius_;
    point_t center_;
  };
}

#endif
