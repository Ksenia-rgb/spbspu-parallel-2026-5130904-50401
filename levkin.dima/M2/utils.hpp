#ifndef UTILS_HPP
#define UTILS_HPP

#include <array>
#include <istream>
#include <vector>

namespace levkin {
  class Circle;
  class Elipse;
  

  struct p_t {
    double x, y;
  };



  struct AnyAllResult {
    size_t any, all = 0;

    AnyAllResult &operator+=(const AnyAllResult &other)
    {
      any += other.any;
      all += other.all;
      return *this;
    }
  };
  struct Rect {
      p_t bottomLeft;
      p_t topRight;
  };

  std::istream &operator>>(std::istream &is, Circle &c);
}
#endif
