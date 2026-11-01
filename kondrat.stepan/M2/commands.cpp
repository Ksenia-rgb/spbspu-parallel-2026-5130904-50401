#include "commands.hpp"
#include <cstddef>
#include <cmath>
#include <iomanip>
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
  constexpr int angle_precision = 3;

  if (words.size() != expected_words)
  {
    throw std::invalid_argument("Invalid parameter count");
  }

  const std::map< std::string, Shape >::const_iterator position = shapes.find(words[1]);

  if (position == shapes.end())
  {
    throw std::invalid_argument("Unknown shape");
  }

  const Shape & shape = position->second;
  output << shape.horizontal_radius;

  if (shape.kind == ShapeKind::Ellipse)
  {
    output << ' ' << shape.vertical_radius;
  }

  output << " (" << shape.x << ' ' << shape.y << ')';

  if (shape.kind == ShapeKind::Ellipse)
  {
    output << ' ' << std::fixed << std::setprecision(angle_precision) << shape.angle;
  }

  output << '\n';
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
