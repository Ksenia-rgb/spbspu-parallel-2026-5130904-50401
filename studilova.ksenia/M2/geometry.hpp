#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <iosfwd>
#include <vector>

namespace studilova
{
  struct circle_t
  {
    int radius;
    int x;
    int y;
  };

  struct rectangle_t
  {
    double min_x;
    double min_y;
    double max_x;
    double max_y;
  };

  std::ostream& operator<<(std::ostream& out, const circle_t& circle);
  std::ostream& operator<<(std::ostream& out, const rectangle_t& rectangle);

  rectangle_t findFrame(const circle_t& circle);
  rectangle_t findFrame(const std::vector< circle_t >& circles);

  bool isInside(double x, double y, const std::vector< circle_t >& circles);
}

#endif
