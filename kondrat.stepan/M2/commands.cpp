#include "commands.hpp"
#include <algorithm>
#include <cstddef>
#include <cmath>
#include <ostream>
#include <stdexcept>
#include "input.hpp"

void kondrat::createShape(
  const std::vector< std::string > & words,
  std::map< std::string, Shape > & shapes)
{
  constexpr size_t circle_words = 5;
  constexpr size_t ellipse_words = 6;
  constexpr size_t parameters_begin = 2;

  if (words.empty() || (words[0] != "circle" && words[0] != "ellipse"))
  {
    throw std::invalid_argument("Invalid shape command");
  }

  const bool is_circle = words[0] == "circle";
  const size_t expected_words = is_circle ? circle_words : ellipse_words;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::string & name = words[1];

  if (!isValidName(name) || shapes.find(name) != shapes.end())
  {
    throw std::invalid_argument("Invalid or duplicate shape name");
  }

  size_t index = parameters_begin;
  const int horizontal_radius = parseInteger(words[index++]);
  const int vertical_radius = is_circle ? horizontal_radius : parseInteger(words[index++]);
  const int x = parseInteger(words[index++]);
  const int y = parseInteger(words[index]);

  if (horizontal_radius <= 0 || vertical_radius <= 0)
  {
    throw std::invalid_argument("Radii must be positive");
  }

  const Shape shape =
  {
    is_circle ? ShapeKind::Circle : ShapeKind::Ellipse,
    horizontal_radius,
    vertical_radius,
    x,
    y,
    0.0
  };

  shapes.emplace(name, shape);
}

void kondrat::showShape(
  const std::vector< std::string > & words,
  const std::map< std::string, Shape > & shapes,
  std::ostream & output)
{
  constexpr size_t expected_words = 2;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::map< std::string, Shape >::const_iterator position = shapes.find(words[1]);

  if (position == shapes.end())
  {
    throw std::invalid_argument("Unknown shape");
  }

  printShape(position->second, output);
}

void kondrat::rotateShape(
  const std::vector< std::string > & words,
  std::map< std::string, Shape > & shapes)
{
  constexpr size_t expected_words = 3;
  constexpr size_t angle_index = 2;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::map< std::string, Shape >::iterator position = shapes.find(words[1]);

  if (position == shapes.end())
  {
    throw std::invalid_argument("Unknown shape");
  }

  const double angle = parseReal(words[angle_index]);
  const double new_angle = position->second.angle + angle;

  if (!std::isfinite(new_angle))
  {
    throw std::invalid_argument("Angle is out of range");
  }

  position->second.angle = new_angle;
}

void kondrat::createSet(
  const std::vector< std::string > & words,
  const std::map< std::string, Shape > & shapes,
  std::map< std::string, std::vector< std::string > > & sets)
{
  constexpr size_t names_begin = 3;
  constexpr size_t size_index = 2;

  if (words.size() < names_begin)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::string & name = words[1];

  if (!isValidName(name) || sets.find(name) != sets.end())
  {
    throw std::invalid_argument("Invalid or duplicate set name");
  }

  const int set_size = parseInteger(words[size_index]);

  if (set_size <= 0 || static_cast< size_t >(set_size) != words.size() - names_begin)
  {
    throw std::invalid_argument("Invalid set size");
  }

  std::vector< std::string > names;

  for (size_t i = names_begin; i < words.size(); ++i)
  {
    if (shapes.find(words[i]) == shapes.end())
    {
      throw std::invalid_argument("Unknown shape");
    }

    names.push_back(words[i]);
  }

  sets.emplace(name, names);
}

void kondrat::showSet(
  const std::vector< std::string > & words,
  const std::map< std::string, Shape > & shapes,
  const std::map< std::string, std::vector< std::string > > & sets,
  std::ostream & output)
{
  constexpr size_t expected_words = 2;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::map< std::string, std::vector< std::string > >::const_iterator position = sets.find(words[1]);

  if (position == sets.end())
  {
    throw std::invalid_argument("Unknown set");
  }

  const std::vector< std::string > & names = position->second;
  output << names.size() << '\n';

  for (const std::string & name : names)
  {
    printShape(shapes.at(name), output);
  }
}

void kondrat::showFrame(
  const std::vector< std::string > & words,
  const std::map< std::string, Shape > & shapes,
  std::ostream & output)
{
  constexpr size_t expected_words = 2;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::map< std::string, Shape >::const_iterator position = shapes.find(words[1]);

  if (position == shapes.end())
  {
    throw std::invalid_argument("Unknown shape");
  }

  printFrame(findFrame(position->second), output);
}

void kondrat::showSetFrame(
  const std::vector< std::string > & words,
  const std::map< std::string, Shape > & shapes,
  const std::map< std::string, std::vector< std::string > > & sets,
  std::ostream & output)
{
  constexpr size_t expected_words = 2;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::map< std::string, std::vector< std::string > >::const_iterator position = sets.find(words[1]);

  if (position == sets.end())
  {
    throw std::invalid_argument("Unknown set");
  }

  const std::vector< std::string > & names = position->second;
  Rectangle rectangle = findFrame(shapes.at(names.front()));

  for (const std::string & name : names)
  {
    const Rectangle frame = findFrame(shapes.at(name));

    rectangle.left = std::min(rectangle.left, frame.left);
    rectangle.right = std::max(rectangle.right, frame.right);
    rectangle.bottom = std::min(rectangle.bottom, frame.bottom);
    rectangle.top = std::max(rectangle.top, frame.top);
  }

  printFrame(rectangle, output);
}
