#include "monteCarlo.hpp"
#include <algorithm>
#include <cstddef>

bool zhuravleva::isInside(const Circle& circle, double x, double y)
{
  const double dx = x - circle.x;
  const double dy = y - circle.y;
  const double rad = circle.radius;
  return dx * dx + dy * dy <= rad * rad;
}

zhuravleva::Bounds zhuravleva::getBounds(const std::vector<Circle>& circles)
{
  const Circle& first = circles.front();
  Bounds bounds {
    static_cast< double >(first.x) - first.radius,
    static_cast< double >(first.x) + first.radius,
    static_cast< double >(first.y) - first.radius,
    static_cast< double >(first.y) + first.radius
  };
  for (std::size_t i = 1; i < circles.size(); i++)
  {
    const Circle& circle = circles[i];
    bounds.minX = std::min(bounds.minX, static_cast< double >(circle.x) - circle.radius);
    bounds.maxX = std::max(bounds.maxX, static_cast< double >(circle.x) + circle.radius);
    bounds.minY = std::min(bounds.minY, static_cast< double >(circle.y) - circle.radius);
    bounds.maxY = std::max(bounds.maxY, static_cast< double >(circle.y) + circle.radius);
  }
  return bounds;
}
