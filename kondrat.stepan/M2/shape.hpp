#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <iosfwd>

namespace kondrat
{
  enum class ShapeKind
  {
    Circle,
    Ellipse
  };

  struct Shape
  {
    ShapeKind kind;
    int horizontal_radius;
    int vertical_radius;
    int x;
    int y;
    double angle;
  };
  void printShape(const Shape & shape, std::ostream & output);
}

#endif
