#include "commands.hpp"
#include "geometry.hpp"
#include "pipe.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <iomanip>
#include <limits>
#include <cerrno>
#include <cstring>
#include <poll.h>

namespace studilova
{
  namespace detail
  {
    void skipSpaces(std::istream& in);
    std::string readArgument(std::istream& in);
    int readInteger(std::istream& in);
    void lineEnd(std::istream& in);
    bool isValidName(const std::string& name);
    void printCalculation(std::ostream& out, const calculation_t& calculation);
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

void studilova::detail::printCalculation(std::ostream& out, const calculation_t& calculation)
{
  if (!calculation.completed)
  {
    out << calculation.processName << " <IN PROGRESS>\n";
    return;
  }

  if (!calculation.success)
  {
    throw std::runtime_error("Calculation failed");
  }

  const auto oldFlags = out.flags();
  const auto oldPrecision = out.precision();

  out << calculation.processName << ' ' << std::fixed << std::setprecision(3) << calculation.milliseconds << ' '
      << calculation.area << '\n';

  out.flags(oldFlags);
  out.precision(oldPrecision);
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

void studilova::spawnProcess(std::istream& in, processes_t& processes)
{
  const std::string name = detail::readArgument(in);
  const std::string seedArgument = detail::readArgument(in);
  detail::lineEnd(in);

  if (!detail::isValidName(name) || seedArgument.front() == '-')
  {
    throw std::invalid_argument("Invalid process parameters");
  }

  std::size_t position = 0;
  unsigned long long seed = 0;

  try
  {
    seed = std::stoull(seedArgument, &position);
  }
  catch (const std::out_of_range&)
  {
    throw std::invalid_argument("Seed is out of range");
  }

  if (position != seedArgument.size() || seed > std::numeric_limits< unsigned int >::max())
  {
    throw std::invalid_argument("Invalid seed");
  }

  if (processes.find(name) != processes.end())
  {
    throw std::invalid_argument("Process already exists");
  }

  const auto inserted = processes.emplace(name, process_t{-1, Descriptor(), Descriptor()});

  try
  {
    inserted.first->second = startProcess(static_cast< unsigned int >(seed), processes);
  }
  catch (...)
  {
    processes.erase(inserted.first);
    throw;
  }
}

void studilova::areaOn(std::istream& in, const circles_t& circles, const sets_t& sets,
    processes_t& processes, calculations_t& calculations)
{
  const std::string processName = detail::readArgument(in);
  const std::string calculationName = detail::readArgument(in);
  const std::string setName = detail::readArgument(in);
  const int threads = detail::readInteger(in);
  const int tries = detail::readInteger(in);
  detail::lineEnd(in);

  if (!detail::isValidName(calculationName) || threads <= 0 || tries <= 0)
  {
    throw std::invalid_argument("Invalid calculation parameters");
  }

  if (calculations.find(calculationName) != calculations.end())
  {
    throw std::invalid_argument("Calculation already exists");
  }

  const auto processFound = processes.find(processName);
  if (processFound == processes.end())
  {
    throw std::invalid_argument("Process does not exist");
  }

  process_t& process = processFound->second;

  if (process.writeDescriptor.get() == -1)
  {
    throw std::invalid_argument("Process is unavailable");
  }

  const auto setFound = sets.find(setName);
  if (setFound == sets.end())
  {
    throw std::invalid_argument("Set does not exist");
  }

  std::vector< circle_t > shapes;
  shapes.reserve(setFound->second.size());

  for (const std::string& name: setFound->second)
  {
    shapes.push_back(circles.at(name));
  }

  const std::size_t calculationId = calculations.size();

  const request_t request{calculationId, static_cast< std::size_t >(threads), static_cast< std::size_t >(tries), shapes.size()};

  const calculation_t calculation{calculationId, processName, false, false, 0.0, 0.0};

  const auto inserted = calculations.emplace(calculationName, calculation);

  try
  {
    sendRequest(process.writeDescriptor.get(), request, shapes);
  }
  catch (...)
  {
    process.writeDescriptor = Descriptor();
    inserted.first->second.completed = true;
    throw;
  }
}

void studilova::waitCalculation(std::istream& in, std::ostream& out, processes_t& processes, calculations_t& calculations)
{
  const std::string name = detail::readArgument(in);
  detail::lineEnd(in);

  const auto found = calculations.find(name);

  if (found == calculations.end())
  {
    throw std::invalid_argument("Calculation does not exist");
  }

  calculation_t& calculation = found->second;

  if (!calculation.completed)
  {
    const auto processFound = processes.find(calculation.processName);

    if (processFound == processes.end())
    {
      throw std::runtime_error("Process does not exist");
    }

    process_t& process = processFound->second;

    while (!calculation.completed)
    {
      receiveResult(process, calculation.processName, calculations);
    }
  }
  detail::printCalculation(out, calculation);
}

void studilova::showStatus(std::istream& in, std::ostream& out, processes_t& processes, calculations_t& calculations)
{
  const std::string name = detail::readArgument(in);
  detail::lineEnd(in);

  const auto found = calculations.find(name);

  if (found == calculations.end())
  {
    throw std::invalid_argument("Calculation does not exist");
  }

  calculation_t& calculation = found->second;

  if (!calculation.completed)
  {
    const auto processFound = processes.find(calculation.processName);

    if (processFound == processes.end())
    {
      throw std::runtime_error("Process does not exist");
    }

    process_t& process = processFound->second;

    while (!calculation.completed)
    {
      if (process.readDescriptor.get() == -1)
      {
        throw std::runtime_error("Process is unavailable");
      }

      pollfd descriptor{};
      descriptor.fd = process.readDescriptor.get();
      descriptor.events = POLLIN;

      const int result = poll(&descriptor, 1, 0);

      if (result == -1)
      {
        throw std::runtime_error(std::strerror(errno));
      }

      if (result == 0)
      {
        break;
      }

      if (descriptor.revents & POLLNVAL)
      {
        throw std::runtime_error("Invalid result descriptor");
      }

      if (descriptor.revents & POLLIN)
      {
        receiveResult(process, calculation.processName, calculations);
      }
      else if (descriptor.revents & POLLHUP)
      {
        receiveResult(process, calculation.processName, calculations);
      }
      else
      {
        throw std::runtime_error("Result pipe error");
      }
    }
  }
  detail::printCalculation(out, calculation);
}
