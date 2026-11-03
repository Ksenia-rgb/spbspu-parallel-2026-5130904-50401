#include "area_calc.hpp"

#include <algorithm>
#include <cmath>
#include <future>
#include <random>
#include <stdexcept>

bool lukashevich::contains(const Shape & shape, double x, double y)
{
  const double delta_x = x - shape.center_x;
  const double delta_y = y - shape.center_y;
  if (shape.circle)
  {
    const double radius = static_cast< double >(shape.radius_x);
    return delta_x * delta_x + delta_y * delta_y <= radius * radius;
  }
  const double cosine = std::cos(shape.angle);
  const double sine = std::sin(shape.angle);
  const double local_x = (delta_x * cosine + delta_y * sine) / shape.radius_x;
  const double local_y = (delta_y * cosine - delta_x * sine) / shape.radius_y;
  return local_x * local_x + local_y * local_y <= 1.0;
}

lukashevich::Bounds lukashevich::getBounds(const std::vector< Shape > & shapes)
{
  if (shapes.empty())
  {
    return {0.0, 0.0, 0.0, 0.0};
  }
  Bounds bounds{};
  bool first = true;
  for (const Shape & shape : shapes)
  {
    const double cosine = std::cos(shape.angle);
    const double sine = std::sin(shape.angle);
    const double half_width = std::hypot(shape.radius_x * cosine, shape.radius_y * sine);
    const double half_height = std::hypot(shape.radius_x * sine, shape.radius_y * cosine);
    const double x = static_cast< double >(shape.center_x);
    const double y = static_cast< double >(shape.center_y);
    const Bounds current{x - half_width, x + half_width, y - half_height, y + half_height};
    if (first)
    {
      bounds = current;
      first = false;
    }
    else
    {
      bounds.min_x = std::min(bounds.min_x, current.min_x);
      bounds.max_x = std::max(bounds.max_x, current.max_x);
      bounds.min_y = std::min(bounds.min_y, current.min_y);
      bounds.max_y = std::max(bounds.max_y, current.max_y);
    }
  }
  return bounds;
}
