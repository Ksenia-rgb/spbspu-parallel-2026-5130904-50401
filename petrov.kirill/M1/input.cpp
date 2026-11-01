#include "input.hpp"

#include <cctype>
#include <cstddef>
#include <istream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "area.hpp"

std::size_t petrov::parseCount(const char *text)
{
  if (*text == '-') {
    throw std::invalid_argument("Negative values are not allowed");
  }
  if (*text == '\0') {
    throw std::invalid_argument("Empty argument");
  }

  const std::size_t base = 10;
  const std::size_t limit = std::numeric_limits< std::size_t >::max();
  std::size_t result = 0;
  for (const char *it = text; *it != '\0'; ++it) {
    if (!std::isdigit(static_cast< unsigned char >(*it))) {
      throw std::invalid_argument("Argument is not a number");
    }
    const std::size_t digit = static_cast< std::size_t >(*it - '0');
    if (result > ((limit - digit) / base)) {
      throw std::overflow_error("Argument is too big");
    }
    result = result * base + digit;
  }
  return result;
}

std::vector< petrov::circle_t > petrov::readCircles(std::istream &in)
{
  std::vector< circle_t > circles;
  while (!(in >> std::ws).eof()) {
    int radius = 0;
    int skipped = 0;
    int x = 0;
    int y = 0;
    if (!(in >> radius >> skipped >> x >> y)) {
      throw std::invalid_argument("Unable to parse figure");
    }
    const circle_t circle = {static_cast< double >(x), static_cast< double >(y), static_cast< double >(radius)};
    circles.push_back(circle);
  }
  if (in.bad()) {
    throw std::runtime_error("Input stream failure");
  }
  return circles;
}
