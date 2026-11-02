#include "commands.hpp"
#include <iostream>
#include <stdexcept>
#include <string>

namespace studilova
{
  namespace detail
  {
    void skipSpaces(std::istream& in);
    std::string readArgument(std::istream& in);
    int readInteger(std::istream& in);
    void requireLineEnd(std::istream& in);
    bool isValidName(const std::string& name);
  }
}

void studilova::createCircle(std::istream& in, circles_t& circles)
{
  const std::string name = detail::readArgument(in);
  const int radius = detail::readInteger(in);
  const int x = detail::readInteger(in);
  const int y = detail::readInteger(in);
  detail::requireLineEnd(in);

  if (!detail::isValidName(name) || radius <= 0)
  {
    throw std::invalid_argument("Invalid circle parameters");
  }

  if (circles.find(name) != circles.end())
  {
    throw std::invalid_argument("Circle already exists");
  }

  const circle_t circle{radius, x, y};
  circles.emplace(name, circle);
}

void studilova::showCircle(std::istream& in, std::ostream& out, const circles_t& circles)
{
  const std::string name = detail::readArgument(in);
  detail::requireLineEnd(in);

  const auto found = circles.find(name);
  if (found == circles.end())
  {
    throw std::invalid_argument("Circle does not exist");
  }

  const circle_t& circle = found->second;
  out << circle.radius << " (" << circle.x << ' ' << circle.y << ")\n";
}
