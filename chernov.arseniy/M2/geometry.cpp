#include "geometry.hpp"

#include <istream>

std::istream & chernov::operator>>(std::istream & input, ellipse_t & ellipse)
{
  std::istream::sentry sentry(input);
  if (!sentry) {
    return input;
  }

  using sep = detail::DelimiterIO;
  double a = 0, b = 0, c = 0;
  input >> a >> sep{' '} >> b >> sep{' '} >> c;

  if (input.peek() == '\n' || input.peek() == EOF) {
    ellipse.center = {.x = b, .y = c};
    ellipse.horizontal_radius = a;
    ellipse.vertical_radius = a;
  } else {
    double d = 0;
    input >> sep{' '} >> d;
    ellipse.center = {.x = c, .y = d};
    ellipse.horizontal_radius = a;
    ellipse.vertical_radius = b;
  }

  return input;
}

std::istream & chernov::operator>>(std::istream & input, set_t & set)
{
  std::istream::sentry sentry(input);
  if (!sentry) {
    return input;
  }

  int set_size = 0;
  input >> set_size;

  if (set_size <= 0) {
    input.setstate(std::ios::failbit);
    return input;
  }

  set.reserve(set_size);
  using sep = detail::DelimiterIO;
  for (int i = 0; i < set_size; i++) {
    std::string name;
    input >> sep{' '} >> name;
  }

  // ...

  return input;
}
