#include "shape.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <ostream>

namespace {
  const int angle_precision = 3;
  const double rounding_error = 1e-9;

  double getHalfWidth(const petrov::Shape& shape)
  {
    const double radius_x = static_cast< double >(shape.radius_x);
    if (!shape.is_ellipse) {
      return radius_x;
    }
    const double radius_y = static_cast< double >(shape.radius_y);
    const double part_x = radius_x * std::cos(shape.angle);
    const double part_y = radius_y * std::sin(shape.angle);
    return std::sqrt(part_x * part_x + part_y * part_y);
  }

  double getHalfHeight(const petrov::Shape& shape)
  {
    const double radius_y = static_cast< double >(shape.radius_y);
    if (!shape.is_ellipse) {
      return radius_y;
    }
    const double radius_x = static_cast< double >(shape.radius_x);
    const double part_x = radius_x * std::sin(shape.angle);
    const double part_y = radius_y * std::cos(shape.angle);
    return std::sqrt(part_x * part_x + part_y * part_y);
  }

  long long roundUp(double value)
  {
    return static_cast< long long >(std::ceil(value - rounding_error));
  }
}

petrov::Frame petrov::getFrame(const Shape& shape)
{
  const long long half_width = roundUp(getHalfWidth(shape));
  const long long half_height = roundUp(getHalfHeight(shape));
  Frame frame{0, 0, 0, 0};
  frame.left = shape.x - half_width;
  frame.bottom = shape.y - half_height;
  frame.right = shape.x + half_width;
  frame.top = shape.y + half_height;
  return frame;
}

petrov::Frame petrov::getFrame(const std::vector< Shape >& shapes)
{
  Frame result = getFrame(shapes[0]);
  for (size_t i = 1; i < shapes.size(); ++i) {
    const Frame frame = getFrame(shapes[i]);
    result.left = std::min(result.left, frame.left);
    result.bottom = std::min(result.bottom, frame.bottom);
    result.right = std::max(result.right, frame.right);
    result.top = std::max(result.top, frame.top);
  }
  return result;
}

bool petrov::isInside(const Shape& shape, double x, double y)
{
  const double dx = x - static_cast< double >(shape.x);
  const double dy = y - static_cast< double >(shape.y);
  const double radius_x = static_cast< double >(shape.radius_x);
  const double radius_y = static_cast< double >(shape.radius_y);
  if (!shape.is_ellipse) {
    return (dx * dx + dy * dy) <= (radius_x * radius_x);
  }
  const double cos_angle = std::cos(shape.angle);
  const double sin_angle = std::sin(shape.angle);
  const double turned_x = dx * cos_angle + dy * sin_angle;
  const double turned_y = dy * cos_angle - dx * sin_angle;
  const double part_x = turned_x / radius_x;
  const double part_y = turned_y / radius_y;
  return (part_x * part_x + part_y * part_y) <= 1.0;
}
