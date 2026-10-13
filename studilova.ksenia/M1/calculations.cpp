#include "calculations.hpp"

bool studilova::containsPoint(const Circle& circle, double x, double y)
{
  double dx = x - circle.x;
  double dy = y - circle.y;

  return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}
