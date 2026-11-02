#include "geometry.hpp"

studilova::rectangle_t studilova::findFrame(const circle_t& circle)
{
  const double x = circle.x;
  const double y = circle.y;
  const double radius = circle.radius;

  return rectangle_t{x - radius, y - radius, x + radius, y + radius};
}
