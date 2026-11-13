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
    double min_x;
    double min_y;
    double max_x;
    double max_y;
  };

  bool containsPoint(const Circle& circle, double x, double y);
  Rectangle findRectangle(const std::vector< Circle >& circles);

  std::pair< size_t, size_t > countHits(
      const std::vector< Circle >& circles, const Rectangle& rectangle, size_t tries, size_t seed);

  std::pair< double, double > calculateAreas(
      const std::vector< Circle >& circles, size_t thread_count, size_t tries, size_t seed);

  size_t parseArgument(const char* text);
}

#endif
