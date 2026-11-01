#include "geometry.hpp"

#include <iomanip>
#include <ostream>

std::ostream& chernov::operator<<(std::ostream& output, const point_t& point)
{
  std::ostream::sentry sentry(output);
  if (!sentry) {
    return output;
  }

  detail::IOguard guard(output);
  output << "(" << point.x << " " << point.y << ")";

  return output;
}

std::ostream& chernov::operator<<(std::ostream& output, const frame_t& frame)
{
  std::ostream::sentry sentry(output);
  if (!sentry) {
    return output;
  }

  detail::IOguard guard(output);
  output << frame.left_bottom << " " << frame.right_top;

  return output;
}

std::ostream& chernov::operator<<(std::ostream& output, const ellipse_t& ellipse)
{
  std::ostream::sentry sentry(output);
  if (!sentry) {
    return output;
  }

  detail::IOguard guard(output);
  if (ellipse.horizontal_radius == ellipse.vertical_radius) {
    output << ellipse.horizontal_radius << " " << ellipse.center;
  } else {
    output << ellipse.horizontal_radius << " " << ellipse.vertical_radius << " " << ellipse.center;
    output << std::fixed << std::setprecision(3) << ellipse.ksi;
  }

  return output;
}

chernov::frame_t chernov::getFrame(const ellipse_t& ellipse)
{
  return {
      .left_bottom = {ellipse.center.x - ellipse.horizontal_radius, ellipse.center.y - ellipse.vertical_radius},
      .right_top = {ellipse.center.x + ellipse.horizontal_radius, ellipse.center.y + ellipse.vertical_radius},
  };
}

chernov::detail::IOguard::IOguard(std::basic_ios< char >& s):
  s_(s),
  width_(s.width()),
  fill_(s.fill()),
  precision_(s.precision()),
  fmt_(s.flags())
{}

chernov::detail::IOguard::~IOguard()
{
  s_.width(width_);
  s_.fill(fill_);
  s_.precision(precision_);
  s_.flags(fmt_);
}
