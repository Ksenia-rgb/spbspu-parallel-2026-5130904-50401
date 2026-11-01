#include "shapes.hpp"

zhuravleva::Rectangle zhuravleva::getBounds(const Shape& shape)
{
  const double min_x = shape.x - shape.rx;
  const double max_x = shape.x + shape.rx;
  const double min_y = shape.y - shape.ry;
  const double max_y = shape.y + shape.ry;

  return Rectangle{min_x, max_x, min_y, max_y};
}
