#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <iosfwd>
#include <vector>

namespace petrov {
  struct Shape {
    bool is_ellipse;
    long long radius_x;
    long long radius_y;
    long long x;
    long long y;
    double angle;
  };

  struct Frame {
    long long left;
    long long bottom;
    long long right;
    long long top;
  };

  Frame getFrame(const Shape& shape);
  Frame getFrame(const std::vector< Shape >& shapes);
  bool isInside(const Shape& shape, double x, double y);
  void printShape(std::ostream& out, const Shape& shape);
  void printFrame(std::ostream& out, const Frame& frame);
}

#endif
