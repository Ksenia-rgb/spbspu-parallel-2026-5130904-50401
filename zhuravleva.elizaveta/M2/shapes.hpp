#ifndef SHAPES_HPP
#define SHAPES_HPP

namespace zhuravleva
{
  enum class ShapeType
  {
    circle,
    ellipse
  };

  struct Shape
  {
    ShapeType type;
    int rx;
    int ry;
    int x;
    int y;
    double angle;
  };
}

#endif
