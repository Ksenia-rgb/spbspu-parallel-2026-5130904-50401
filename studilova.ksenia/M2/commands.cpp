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
    out << calculation.process_name << " <IN PROGRESS>\n";
    return;
  }

  if (!calculation.success)
  {
    throw std::runtime_error("Calculation failed");
  }

  const auto oldFlags = out.flags();
  const auto oldPrecision = out.precision();

  out << calculation.process_name << ' ' << std::fixed << std::setprecision(3) << calculation.milliseconds << ' '
      << calculation.area << '\n';

  out.flags(oldFlags);
  out.precision(oldPrecision);
}

void studilova::createCircle(std::istream& in, std::ostream&, State& state)
{
  circles_t& circles = state.circles;
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

void studilova::showCircle(std::istream& in, std::ostream& out, State& state)
{
  const circles_t& circles = state.circles;
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

void studilova::createSet(std::istream& in, std::ostream&, State& state)
{
  const circles_t& circles = state.circles;
  sets_t& sets = state.sets;
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

  circle_set_t circle_set;

  for (int i = 0; i < size; ++i)
  {
    const std::string circle_name = detail::readArgument(in);

    if (circles.find(circle_name) == circles.end())
    {
      throw std::invalid_argument("Circle does not exist");
    }

    circle_set.push_back(circle_name);
  }

  detail::lineEnd(in);
  sets.emplace(name, circle_set);
}

void studilova::showSet(std::istream& in, std::ostream& out, State& state)
{
  const circles_t& circles = state.circles;
  const sets_t& sets = state.sets;
  const std::string name = detail::readArgument(in);
  detail::lineEnd(in);

  const auto found = sets.find(name);
  if (found == sets.end())
  {
    throw std::invalid_argument("Set does not exist");
  }

  const circle_set_t& circle_set = found->second;

  for (const std::string& circle_name : circle_set)
  {
    const circle_t& circle = circles.at(circle_name);
    out << circle << '\n';
  }
}

void studilova::showFrame(std::istream& in, std::ostream& out, State& state)
{
  const circles_t& circles = state.circles;
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

void studilova::showSetFrame(std::istream& in, std::ostream& out, State& state)
{
  const circles_t& circles = state.circles;
  const sets_t& sets = state.sets;
  const std::string name = detail::readArgument(in);
  detail::lineEnd(in);

  const auto found = sets.find(name);
  if (found == sets.end())
  {
    throw std::invalid_argument("Set does not exist");
  }

  std::vector< circle_t > shapes;
  shapes.reserve(found->second.size());

  for (const std::string& circle_name : found->second)
  {
    shapes.push_back(circles.at(circle_name));
  }

  const rectangle_t frame = findFrame(shapes);

  out << frame << '\n';
}

void studilova::spawnProcess(std::istream& in, std::ostream&, State& state)
{
  processes_t& processes = state.processes;
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

void studilova::areaOn(std::istream& in, std::ostream&, State& state)
{
  const circles_t& circles = state.circles;
  const sets_t& sets = state.sets;
  processes_t& processes = state.processes;
  calculations_t& calculations = state.calculations;
  const std::string process_name = detail::readArgument(in);
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

  const auto processFound = processes.find(process_name);
  if (processFound == processes.end())
  {
    throw std::invalid_argument("Process does not exist");
  }

  process_t& process = processFound->second;

  if (process.write_descriptor.get() == -1)
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

  for (const std::string& name : setFound->second)
  {
    shapes.push_back(circles.at(name));
  }

  const std::size_t calculation_id = calculations.size();

  const request_t request{
      calculation_id, static_cast< std::size_t >(threads), static_cast< std::size_t >(tries), shapes.size()};

  const calculation_t calculation{calculation_id, process_name, false, false, 0.0, 0.0};

  const auto inserted = calculations.emplace(calculationName, calculation);

  try
  {
    sendRequest(process.write_descriptor.get(), request, shapes);
  }
  catch (...)
  {
    process.write_descriptor = Descriptor();
    inserted.first->second.completed = true;
    throw;
  }
}

void studilova::waitCalculation(std::istream& in, std::ostream& out, State& state)
{
  processes_t& processes = state.processes;
  calculations_t& calculations = state.calculations;
  const std::string name = detail::readArgument(in);
  detail::lineEnd(in);

  const auto found = calculations.find(name);

  if (found == calculations.end())
  {
    throw std::invalid_argument("Calculation does not exist");
  }

  const calculation_t& calculation = found->second;

  if (!calculation.completed)
  {
    const auto processFound = processes.find(calculation.process_name);

    if (processFound == processes.end())
    {
      throw std::runtime_error("Process does not exist");
    }

    process_t& process = processFound->second;

    while (!calculation.completed)
    {
      receiveResult(process, calculation.process_name, calculations);
    }
  }
  detail::printCalculation(out, calculation);
}

void studilova::showStatus(std::istream& in, std::ostream& out, State& state)
{
  processes_t& processes = state.processes;
  calculations_t& calculations = state.calculations;
  const std::string name = detail::readArgument(in);
  detail::lineEnd(in);

  const auto found = calculations.find(name);

  if (found == calculations.end())
  {
    throw std::invalid_argument("Calculation does not exist");
  }

  const calculation_t& calculation = found->second;

  if (!calculation.completed)
  {
    const auto processFound = processes.find(calculation.process_name);

    if (processFound == processes.end())
    {
      throw std::runtime_error("Process does not exist");
    }

    process_t& process = processFound->second;

    while (!calculation.completed)
    {
      if (process.read_descriptor.get() == -1)
      {
        throw std::runtime_error("Process is unavailable");
      }

      pollfd descriptor{};
      descriptor.fd = process.read_descriptor.get();
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
        receiveResult(process, calculation.process_name, calculations);
      }
      else if (descriptor.revents & POLLHUP)
      {
        receiveResult(process, calculation.process_name, calculations);
      }
      else
      {
        throw std::runtime_error("Result pipe error");
      }
    }
  }
  detail::printCalculation(out, calculation);
}
