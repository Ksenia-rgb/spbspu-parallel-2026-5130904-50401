#include "shapes.hpp"
#include <algorithm>

zharov::Frame zharov::getFrame(const Circle& shape)
{
  const Point left_lower{shape.center.x - shape.r, shape.center.y - shape.r};
  const Point right_upper{shape.center.x + shape.r, shape.center.y + shape.r};
  return {left_lower, right_upper};
}

zharov::Frame zharov::getFrame(const std::vector< Circle >& shapes)
{
  Frame res = getFrame(shapes.front());
  for (auto i = shapes.cbegin() + 1; i != shapes.cend(); ++i)
  {
    const Frame cur = getFrame(*i);
    res.left_lower.x = std::min(cur.left_lower.x, res.left_lower.x);
    res.left_lower.y = std::min(cur.left_lower.y, res.left_lower.y);
    res.right_upper.x = std::max(cur.right_upper.x, res.right_upper.x);
    res.right_upper.y = std::max(cur.right_upper.y, res.right_upper.y);
  }
  return res;
}

bool zharov::isInside(double x, double y, const Circle& shape)
{
  const double dx = x - shape.center.x;
  const double dy = y - shape.center.y;
  const double r = shape.r;
  return dx * dx + dy * dy <= r * r;
}
