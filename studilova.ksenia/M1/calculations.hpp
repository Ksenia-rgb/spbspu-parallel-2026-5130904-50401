#ifndef CALCULATIONS_HPP
#define CALCULATIONS_HPP

#include <vector>
#include <utility>
#include <cstddef>

namespace studilova
{
  struct Circle
  {
    double radius;
    double x;
    double y;
  };

  struct Rectangle
  {
    double minX;
    double minY;
    double maxX;
    double maxY;
  };

  bool containsPoint(const Circle& circle, double x, double y);
  Rectangle findRectangle(const std::vector< Circle >& circles);

  std::pair< size_t, size_t > countHits(const std::vector< Circle >& circles,
      const Rectangle& rectangle, size_t tries, size_t seed);

  std::pair< double, double > calculateAreas(const std::vector< Circle >& circles,
      size_t threadCount, size_t tries, size_t seed);

  size_t parseArgument(const char* text);
}

#endif
