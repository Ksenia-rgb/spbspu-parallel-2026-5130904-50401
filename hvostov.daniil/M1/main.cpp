#include <algorithm>
#include <cstddef>
#include <iostream>
#include <limits>
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

bool hvostov::isInside(double px, double py, const shape_t& s)
{
  double dx = (px - s.centerx) / s.hor_radius;
  double dy = (py - s.centery) / s.vert_radius;
  return (dx * dx + dy * dy) <= 1.0;
}

size_t hvostov::countInsides(double px, double py, const std::vector< shape_t >& shapes)
{
  return std::count_if(shapes.begin(), shapes.end(), [&](const shape_t& s) { return isInside(px, py, s); });
}

hvostov::rect_t hvostov::getRect(const std::vector< shape_t >& shapes)
{
  const double inf = std::numeric_limits< double >::infinity();
  rect_t rect = {inf, -inf, inf, -inf};
  for (const shape_t& s : shapes) {
    rect.minX = std::min(rect.minX, s.centerx - s.hor_radius);
    rect.maxX = std::max(rect.maxX, s.centerx + s.hor_radius);
    rect.minY = std::min(rect.minY, s.centery - s.vert_radius);
    rect.maxY = std::max(rect.maxY, s.centery + s.vert_radius);
  }
  return rect;
}
