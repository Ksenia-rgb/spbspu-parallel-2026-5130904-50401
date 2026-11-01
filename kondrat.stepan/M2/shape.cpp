#include "shape.hpp"
#include <iomanip>
#include <ostream>

void kondrat::printShape(const Shape & shape, std::ostream & output)
{
  constexpr int angle_precision = 3;

  output << shape.horizontal_radius;

  if (shape.kind == ShapeKind::Ellipse)
  {
    output << ' ' << shape.vertical_radius;
  }

  output << " (" << shape.x << ' ' << shape.y << ')';

  if (shape.kind == ShapeKind::Ellipse)
  {
    output << ' ' << std::fixed << std::setprecision(angle_precision) << shape.angle;
  }

  output << '\n';
}
