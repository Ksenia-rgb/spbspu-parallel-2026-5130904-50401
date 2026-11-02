#include "commands.hpp"
#include "help_functions.hpp"
#include <iostream>

bool donkeev::validateCircle(const std::vector<std::string>& tokens)
{
  bool isGood = true;
  isGood = isGood && tokens.size() == 5;
  isGood = isGood && isValidName(tokens[1]);
  isGood = isGood && isPositiveNumber(tokens[2]);
  isGood = isGood && isNumber(tokens[3]);
  isGood = isGood && isNumber(tokens[4]);

  return isGood;
}

bool donkeev::validateSet(const std::vector<std::string>& tokens)
{
  if (tokens.size() < 3 || !isValidName(tokens[1]) || !isPositiveNumber(tokens[2]))
  {
    return false;
  }
  try
  {
    size_t setSize = std::stoul(tokens[2]);
    return tokens.size() == 3 + setSize;
  }
  catch (...)
  {
    return false;
  }
}

bool donkeev::validateTwoTokens(const std::vector<std::string>& tokens)
{
  return tokens.size() == 2 && isValidName(tokens[1]);
}

bool donkeev::validateSpawn(const std::vector<std::string>& tokens)
{
  return tokens.size() == 3 && isValidName(tokens[1]) && isNumber(tokens[2]);
}

bool donkeev::validateAreaOn(const std::vector<std::string>& tokens)
{
  bool isGood = true;
  isGood = isGood && tokens.size() == 6;
  isGood = isGood && isValidName(tokens[1]);
  isGood = isGood && isValidName(tokens[2]);
  isGood = isGood && isValidName(tokens[3]);
  isGood = isGood && isPositiveNumber(tokens[4]);
  isGood = isGood && isPositiveNumber(tokens[5]);

  return isGood;
}

void donkeev::executeCircle(ShapeManager& manager, const std::vector<std::string>& tokens)
{
  manager.addCircle(tokens[1], std::stoi(tokens[2]), std::stoi(tokens[3]), std::stoi(tokens[4]));
}

void donkeev::executeSet(ShapeManager& manager, const std::vector<std::string>& tokens)
{
  size_t size = std::stoul(tokens[2]);
  std::vector<std::string> shapeNames(tokens.begin() + 3, tokens.begin() + 3 + size);
  manager.addSet(tokens[1], shapeNames);
}

void donkeev::executeShow(ShapeManager& manager, const std::vector<std::string>& tokens)
{
  const auto& circle = manager.getCircle(tokens[1]);
  auto center = circle.getCenter();
  std::cout << circle.getRadius() << " (" << center.x_ << " " << center.y_ << ")\n";
}

void donkeev::executeShowSet(ShapeManager& manager, const std::vector<std::string>& tokens)
{
  const auto& shapeSet = manager.getSet(tokens[1]);
  
  for (const auto& shapeName : shapeSet.shape_names_)
  {
    const auto& circle = manager.getCircle(shapeName);
    auto center = circle.getCenter();
    std::cout << circle.getRadius() << " (" << center.x_ << " " << center.y_ << ")\n";
  }
}

void donkeev::executeFrame(ShapeManager& manager, const std::vector<std::string>& tokens)
{
  const auto& circle = manager.getCircle(tokens[1]);
  donkeev::rectangle_t bbox = circle.getBoundingBox();
  std::cout << "(" << bbox.bottom_left_.x_ << " " << bbox.bottom_left_.y_ << ") "
            << "(" << bbox.top_right_.x_ << " " << bbox.top_right_.y_ << ")\n";
}

void donkeev::executeFrameSet(ShapeManager& manager, const std::vector<std::string>& tokens)
{
  donkeev::rectangle_t bbox = manager.getSetBoundingBox(tokens[1]);
  std::cout << "(" << bbox.bottom_left_.x_ << " " << bbox.bottom_left_.y_ << ") "
            << "(" << bbox.top_right_.x_ << " " << bbox.top_right_.y_ << ")\n";
}
