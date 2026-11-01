#include "shape.hpp"
#include <cmath>
#include <iomanip>
#include <limits>
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

kondrat::Rectangle kondrat::findFrame(const Shape & shape)
{
  const double a = shape.horizontal_radius;
  const double b = shape.vertical_radius;
  double half_width = a;
  double half_height = a;

  if (shape.kind == ShapeKind::Ellipse)
  {
    const double cosine = std::cos(shape.angle);
    const double sine = std::sin(shape.angle);

    half_width = std::hypot(a * cosine, b * sine);
    half_height = std::hypot(a * sine, b * cosine);
  }

  return
  {
    shape.x - half_width,
    shape.x + half_width,
    shape.y - half_height,
    shape.y + half_height
  };
}

void kondrat::printFrame(const Rectangle & rectangle, std::ostream & output)
{
  output << std::defaultfloat << std::setprecision(std::numeric_limits< double >::max_digits10);
  output << '(' << rectangle.left << ' ' << rectangle.bottom << ") ("
    << rectangle.right << ' ' << rectangle.top << ")\n";
}
