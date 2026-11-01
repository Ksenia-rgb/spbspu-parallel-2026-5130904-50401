#include "commands.hpp"
#include <cstddef>
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
