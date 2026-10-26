#include "shape.hpp"

sedov::Box sedov::boundingBox(const Shape & shape)
{
  return Box{shape.x - shape.r, shape.x + shape.r, shape.y - shape.r, shape.y + shape.r};
}

sedov::Box sedov::boundingBox(const std::vector< Shape > & shapes)
{
  Box box = boundingBox(shapes.front());
  for (std::size_t i = 1; i < shapes.size(); ++i) {
    const Box b = boundingBox(shapes[i]);
    box.x_min = std::min(box.x_min, b.x_min);
    box.x_max = std::max(box.x_max, b.x_max);
    box.y_min = std::min(box.y_min, b.y_min);
    box.y_max = std::max(box.y_max, b.y_max);
  }
  return box;
}
