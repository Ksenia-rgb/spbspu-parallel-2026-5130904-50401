#include "geometry.hpp"

#include <algorithm>
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

chernov::frame_t chernov::getFrame(const std::vector< ellipse_t >& ellipses)
{
  auto cmp_min_x = [](const ellipse_t& a, const ellipse_t& b)
  {
    return a.center.x - a.horizontal_radius < b.center.x - b.horizontal_radius;
  };
  auto cmp_min_y = [](const ellipse_t& a, const ellipse_t& b)
  {
    return a.center.y - a.vertical_radius < b.center.y - b.vertical_radius;
  };
  auto cmp_max_x = [](const ellipse_t& a, const ellipse_t& b)
  {
    return a.center.x + a.horizontal_radius < b.center.x + b.horizontal_radius;
  };
  auto cmp_max_y = [](const ellipse_t& a, const ellipse_t& b)
  {
    return a.center.y + a.vertical_radius < b.center.y + b.vertical_radius;
  };

  const auto it_min_x = std::min_element(ellipses.begin(), ellipses.end(), cmp_min_x);
  const auto it_min_y = std::min_element(ellipses.begin(), ellipses.end(), cmp_min_y);
  const auto it_max_x = std::max_element(ellipses.begin(), ellipses.end(), cmp_max_x);
  const auto it_max_y = std::max_element(ellipses.begin(), ellipses.end(), cmp_max_y);

  const double min_x = it_min_x->center.x - it_min_x->horizontal_radius;
  const double min_y = it_min_y->center.y - it_min_y->vertical_radius;
  const double max_x = it_max_x->center.x + it_max_x->horizontal_radius;
  const double max_y = it_max_y->center.y + it_max_y->vertical_radius;

  const frame_t frame{.left_bottom = {.x = min_x, .y = min_y}, .right_top = {.x = max_x, .y = max_y}};
  return frame;
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
