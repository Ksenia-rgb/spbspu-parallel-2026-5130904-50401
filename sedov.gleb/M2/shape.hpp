#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <map>
#include <string>
#include <vector>
#include <cstddef>

namespace sedov {
  struct Shape {
    long long r, x, y;
  };

  struct Box {
    long long x_min, x_max, y_min, y_max;
  };

  struct Result {
    size_t hits_union;
    size_t hits_inter;
  };

  using ShapeMap = std::map< std::string, Shape >;
  using SetMap = std::map< std::string, std::vector< std::string > >;

  Box boundingBox(const Shape & shape);
  Box boundingBox(const std::vector< Shape > & shapes);
  bool isInside(double dx, double dy, double r);
  Result calc(const std::vector< Shape > & shapes, const Box & box, size_t tests, size_t seed);
  Result countTotalHits(const std::vector< Shape > & shapes, const Box & box, size_t threads, size_t tries,
    size_t seed);
}

#endif
