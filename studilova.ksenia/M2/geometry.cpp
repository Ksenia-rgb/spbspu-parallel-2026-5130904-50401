#include "geometry.hpp"

#include <algorithm>
#include <stdexcept>

studilova::rectangle_t studilova::findFrame(const circle_t& circle)
{
  const double x = circle.x;
  const double y = circle.y;
  const double radius = circle.radius;

  return rectangle_t{x - radius, y - radius, x + radius, y + radius};
}

studilova::rectangle_t studilova::findFrame(const std::vector< circle_t >& circles)
{
  if (circles.empty())
  {
    throw std::invalid_argument("No bounds for empty set");
  }

  rectangle_t frame = findFrame(circles.front());
  for (std::size_t i = 1; i < circles.size(); ++i)
  {
    const rectangle_t current = findFrame(circles[i]);

    frame.minX = std::min(frame.minX, current.minX);
    frame.minY = std::min(frame.minY, current.minY);
    frame.maxX = std::max(frame.maxX, current.maxX);
    frame.maxY = std::max(frame.maxY, current.maxY);
  }
  return frame;
}
