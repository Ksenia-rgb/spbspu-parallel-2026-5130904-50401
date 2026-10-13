#include "calculations.hpp"

#include <algorithm>

bool studilova::containsPoint(const Circle& circle, double x, double y)
{
  double dx = x - circle.x;
  double dy = y - circle.y;

  return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

studilova::Rectangle studilova::findRectangle(const std::vector< Circle >& circles)
{
  if (circles.empty())
  {
    return Rectangle{0, 0, 0, 0};
  }

  const Circle& first = circles.front();
  double minX = first.x - first.radius;
  double minY = first.y - first.radius;
  double maxX = first.x + first.radius;
  double maxY = first.y + first.radius;
  Rectangle rectangle = {minX, minY, maxX, maxY};

  for (const Circle& circle : circles)
  {
    rectangle.minX = std::min(rectangle.minX, circle.x - circle.radius);
    rectangle.minY = std::min(rectangle.minY, circle.y - circle.radius);
    rectangle.maxX = std::max(rectangle.maxX, circle.x + circle.radius);
    rectangle.maxY = std::max(rectangle.maxY, circle.y + circle.radius);
  }

  return rectangle;
}
