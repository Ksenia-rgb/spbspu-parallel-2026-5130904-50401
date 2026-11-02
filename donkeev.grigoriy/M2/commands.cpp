#include "commands.hpp"
#include "help_functions.hpp"
#include <iostream>
#include <stdexcept>

bool donkeev::validateCircle(const std::vector< std::string >& tokens)
{
  return tokens.size() == 5 && isValidName(tokens[1]) && isPositiveNumber(tokens[2]) && isNumber(tokens[3])
      && isNumber(tokens[4]);
}

bool donkeev::validateSet(const std::vector< std::string >& tokens)
{
  if (tokens.size() < 3 || !isValidName(tokens[1]) || !isPositiveNumber(tokens[2]))
  {
    return false;
  }
  try
  {
    size_t setSize = std::stoul(tokens[2]);
    if (tokens.size() != 3 + setSize)
    {
      return false;
    }
    for (size_t i = 3; i < tokens.size(); ++i)
    {
      if (!isValidName(tokens[i]))
      {
        return false;
      }
    }
    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool donkeev::validateTwoTokens(const std::vector< std::string >& tokens)
{
  return tokens.size() == 2 && isValidName(tokens[1]);
}

bool donkeev::validateSpawn(const std::vector< std::string >& tokens)
{
  return tokens.size() == 3 && isValidName(tokens[1]) && isNumber(tokens[2]);
}

bool donkeev::validateAreaOn(const std::vector< std::string >& tokens)
{
  return tokens.size() == 6 && isValidName(tokens[1]) && isValidName(tokens[2]) && isValidName(tokens[3])
      && isPositiveNumber(tokens[4]) && isPositiveNumber(tokens[5]);
}

void donkeev::executeCircle(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  manager.addCircle(tokens[1], std::stoi(tokens[2]), std::stoi(tokens[3]), std::stoi(tokens[4]));
}

void donkeev::executeSet(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  size_t size = std::stoul(tokens[2]);
  std::vector< std::string > shapeNames(tokens.begin() + 3, tokens.begin() + 3 + size);
  manager.addSet(tokens[1], shapeNames);
}

void donkeev::executeShow(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  const auto& circle = manager.getCircle(tokens[1]);
  auto center = circle.getCenter();
  std::cout << circle.getRadius() << " (" << center.x_ << " " << center.y_ << ")\n";
}

void donkeev::executeShowSet(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  const auto& shapeSet = manager.getSet(tokens[1]);
  for (const auto& shapeName : shapeSet.shape_names_)
  {
    const auto& circle = manager.getCircle(shapeName);
    auto center = circle.getCenter();
    std::cout << circle.getRadius() << " (" << center.x_ << " " << center.y_ << ")\n";
  }
}

void donkeev::executeFrame(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  const auto& circle = manager.getCircle(tokens[1]);
  rectangle_t bbox = circle.getBoundingBox();
  std::cout << "(" << bbox.bottom_left_.x_ << " " << bbox.bottom_left_.y_ << ") "
            << "(" << bbox.top_right_.x_ << " " << bbox.top_right_.y_ << ")\n";
}

void donkeev::executeFrameSet(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  rectangle_t bbox = manager.getSetBoundingBox(tokens[1]);
  std::cout << "(" << bbox.bottom_left_.x_ << " " << bbox.bottom_left_.y_ << ") "
            << "(" << bbox.top_right_.x_ << " " << bbox.top_right_.y_ << ")\n";
}

void donkeev::executeSpawn(ShapeManager&, ProcessManager& procManager, const std::vector< std::string >& tokens)
{
  const std::string& name = tokens[1];
  long long seed_input = std::stoll(tokens[2]);
  if (seed_input < 0)
  {
    throw std::invalid_argument("Seed cannot be negative");
  }

  if (procManager.hasProcess(name))
  {
    throw std::invalid_argument("Process already exists");
  }

  const size_t seed = static_cast< size_t >(seed_input);
  std::vector< Process > existing_processes = procManager.getAllProcesses();

  Process process = spawnProcess(seed, existing_processes);
  try
  {
    procManager.addProcess(name, process);
  }
  catch (...)
  {
    closeProcess(process);
    throw;
  }
}

void donkeev::executeAreaOn(
    ShapeManager& manager, ProcessManager& procManager, const std::vector< std::string >& tokens)
{
  const std::string& processName = tokens[1];
  const std::string& calcName = tokens[2];
  const std::string& setName = tokens[3];
  size_t threads = std::stoul(tokens[4]);
  size_t tries = std::stoul(tokens[5]);

  procManager.startCalculation(processName, calcName, setName, manager, threads, tries);
}

void donkeev::executeStatus(ShapeManager&, ProcessManager& procManager, const std::vector< std::string >& tokens)
{
  procManager.printStatus(tokens[1], std::cout);
}

void donkeev::executeWait(ShapeManager&, ProcessManager& procManager, const std::vector< std::string >& tokens)
{
  procManager.waitCalculation(tokens[1], std::cout);
}
