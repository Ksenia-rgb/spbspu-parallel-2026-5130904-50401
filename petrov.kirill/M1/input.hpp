#ifndef INPUT_HPP
#define INPUT_HPP

#include <iosfwd>
#include <vector>

#include "area.hpp"

namespace petrov {
  size_t parseCount(const char *text);
  std::vector< circle_t > readCircles(std::istream &in);
}

#endif
