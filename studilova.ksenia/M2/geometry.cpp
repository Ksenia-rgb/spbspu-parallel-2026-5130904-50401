#include "geometry.hpp"

#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <ostream>

std::ostream& studilova::operator<<(std::ostream& out, const circle_t& circle)
{
  out << circle.radius << " (" << circle.x << ' ' << circle.y << ')';
  return out;
}

std::ostream& studilova::operator<<(std::ostream& out, const rectangle_t& rectangle)
{
  const std::ios_base::fmtflags oldFlags = out.flags();
  const std::streamsize oldPrecision = out.precision();

  out << std::fixed << std::setprecision(0);
  out << '(' << rectangle.minX << ' ' << rectangle.minY << ") (";
  out << rectangle.maxX << ' ' << rectangle.maxY << ')';

  out.flags(oldFlags);
  out.precision(oldPrecision);

  return out;
}

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
