#include "commands.hpp"
#include "geometry.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <iomanip>

namespace studilova
{
  namespace detail
  {
    void skipSpaces(std::istream& in);
    std::string readArgument(std::istream& in);
    int readInteger(std::istream& in);
    void lineEnd(std::istream& in);
    bool isValidName(const std::string& name);
  }
}

void studilova::detail::skipSpaces(std::istream& in)
{
  while (in.peek() == ' ' || in.peek() == '\t' || in.peek() == '\r')
  {
    in.get();
  }
}

std::string studilova::detail::readArgument(std::istream& in)
{
  skipSpaces(in);

  if (in.peek() == '\n' || in.peek() == EOF)
  {
    throw std::invalid_argument("Missing argument");
  }

  std::string argument;
  if (!(in >> argument))
  {
    throw std::invalid_argument("Cannot read argument");
  }
  return argument;
}

int studilova::detail::readInteger(std::istream& in)
{
  const std::string argument = readArgument(in);
  std::size_t position = 0;
  int value = 0;

  try
  {
    value = std::stoi(argument, &position);
  }
  catch (const std::out_of_range&)
  {
    throw std::invalid_argument("Integer is out of range");
  }

  if (position != argument.size())
  {
    throw std::invalid_argument("Invalid integer");
  }
  return value;
}

void studilova::detail::lineEnd(std::istream& in)
{
  skipSpaces(in);

  if (in.peek() != '\n' && in.peek() != EOF)
  {
    throw std::invalid_argument("Too many arguments");
  }
}

bool studilova::detail::isValidName(const std::string& name)
{
  if (name.empty())
  {
    return false;
  }

  const char first = name.front();
  return (first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z');
}

void studilova::createCircle(std::istream& in, circles_t& circles)
{
  const std::string name = detail::readArgument(in);
  const int radius = detail::readInteger(in);
  const int x = detail::readInteger(in);
  const int y = detail::readInteger(in);
  detail::lineEnd(in);

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
  detail::lineEnd(in);

  const auto found = circles.find(name);
  if (found == circles.end())
  {
    throw std::invalid_argument("Circle does not exist");
  }

  const circle_t& circle = found->second;
  out << circle << '\n';
}

void studilova::createSet(std::istream& in, const circles_t& circles, sets_t& sets)
{
  const std::string name = detail::readArgument(in);
  const int size = detail::readInteger(in);

  if (!detail::isValidName(name) || size <= 0)
  {
    throw std::invalid_argument("Invalid set parameters");
  }

  if (sets.find(name) != sets.end())
  {
    throw std::invalid_argument("Set already exists");
  }

  circle_set_t circleSet;

  for (int i = 0; i < size; ++i)
  {
    const std::string circleName = detail::readArgument(in);

    if (circles.find(circleName) == circles.end())
    {
      throw std::invalid_argument("Circle does not exist");
    }

    circleSet.push_back(circleName);
  }

  detail::lineEnd(in);
  sets.emplace(name, circleSet);
}

void studilova::showSet(std::istream& in, std::ostream& out, const circles_t& circles, const sets_t& sets)
{
  const std::string name = detail::readArgument(in);
  detail::lineEnd(in);

  const auto found = sets.find(name);
  if (found == sets.end())
  {
    throw std::invalid_argument("Set does not exist");
  }

  const circle_set_t& circleSet = found->second;

  for (const std::string& circleName: circleSet)
  {
    const circle_t& circle = circles.at(circleName);
    out << circle << '\n';
  }
}

void studilova::showFrame(std::istream& in, std::ostream& out, const circles_t& circles)
{
  const std::string name = detail::readArgument(in);
  detail::lineEnd(in);

  const auto found = circles.find(name);
  if (found == circles.end())
  {
    throw std::invalid_argument("Circle does not exist");
  }

  const rectangle_t frame = findFrame(found->second);

  out << frame << '\n';
}

void studilova::showSetFrame(std::istream& in, std::ostream& out, const circles_t& circles, const sets_t& sets)
{
  const std::string name = detail::readArgument(in);
  detail::lineEnd(in);

  const auto found = sets.find(name);
  if (found == sets.end())
  {
    throw std::invalid_argument("Set does not exist");
  }

  std::vector< circle_t > shapes;
  shapes.reserve(found->second.size());

  for (const std::string& circleName: found->second)
  {
    shapes.push_back(circles.at(circleName));
  }

  const rectangle_t frame = findFrame(shapes);

  out << frame << '\n';
}
