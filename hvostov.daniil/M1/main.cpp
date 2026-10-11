#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

namespace hvostov {
  struct shape_t {
    double hor_radius, vert_radius, centerx, centery;
  };

  struct rect_t {
    double minX, maxX, minY, maxY;
  };

  bool isInside(double px, double py, const shape_t& s);
  size_t countInsides(double px, double py, const std::vector< shape_t >& shapes);
  rect_t getRect(const std::vector< shape_t >& shapes);
  std::pair< size_t, size_t > calc(const std::vector< shape_t >& shapes, rect_t rect, size_t tries, size_t seed);
  std::pair< double, double > area(const std::vector< shape_t >& shapes, size_t threads, size_t tries, size_t seed);
  std::vector< shape_t > inputShapes(std::istream& in);
}

int main(int argc, char** argv)
{}
