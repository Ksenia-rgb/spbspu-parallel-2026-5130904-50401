#include "shape.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <ostream>

namespace {
  const int angle_precision = 3;
  const double rounding_error = 1e-9;

  double getHalfWidth(const petrov::Shape& shape)
  {
    const double radius_x = static_cast< double >(shape.radius_x);
    if (!shape.is_ellipse) {
      return radius_x;
    }
    const double radius_y = static_cast< double >(shape.radius_y);
    const double part_x = radius_x * std::cos(shape.angle);
    const double part_y = radius_y * std::sin(shape.angle);
    return std::sqrt(part_x * part_x + part_y * part_y);
  }
}
