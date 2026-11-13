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
  out << '(' << rectangle.min_x << ' ' << rectangle.min_y << ") (";
  out << rectangle.max_x << ' ' << rectangle.max_y << ')';

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

    frame.min_x = std::min(frame.min_x, current.min_x);
    frame.min_y = std::min(frame.min_y, current.min_y);
    frame.max_x = std::max(frame.max_x, current.max_x);
    frame.max_y = std::max(frame.max_y, current.max_y);
  }
  return frame;
}

bool studilova::isInside(double x, double y, const std::vector< circle_t >& circles)
{
  for (const circle_t& circle : circles)
  {
    const double dx = x - circle.x;
    const double dy = y - circle.y;
    const double radius = circle.radius;

    if (dx * dx + dy * dy <= radius * radius)
    {
      return true;
    }
  }
  return false;
}
