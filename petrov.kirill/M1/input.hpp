#ifndef INPUT_HPP
#define INPUT_HPP

#include <cstddef>
#include <iosfwd>
#include <vector>

#include "area.hpp"

namespace petrov {
  std::size_t parseCount(const char *text);
  std::vector< circle_t > readCircles(std::istream &in);
}

#endif
