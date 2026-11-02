#include "commands.hpp"
#include <string>
#include <stdexcept>
#include "parser.hpp"

void madieva::addCircle(madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 5)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const long long & r = parseInteger(args[2]);
  const long long & x = parseInteger(args[3]);
  const long long & y = parseInteger(args[4]);

  Circle circle(r, x, y);

  auto it = state.circles.find(name);
  if (it != state.circles.end())
  {
    throw std::invalid_argument("the name of the circle exists");
  }
  state.circles.insert({name, circle});
}

void madieva::addSet(madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() < 3)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const long long count = parseInteger(args[2]);

  if (count <= 0)
  {
    throw std::invalid_argument("incorrect count");
  }

  if (args.size() != 3 + static_cast< std::size_t >(count))
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const auto it_name = state.sets.find(name);
  if (it_name != state.sets.end())
  {
    throw std::invalid_argument("the name of the set exists");
  }

  std::vector< const Circle * > shapes;
  shapes.reserve(count);

  for (std::size_t i = 3; i < args.size(); ++i)
  {
    const std::string & circle_name = args[i];
    const auto it = state.circles.find(circle_name);
    if (it == state.circles.end())
    {
      throw std::invalid_argument("the circle name does not exist");
    }
    shapes.push_back(&it->second);
  }

  state.sets.insert({name, shapes});
}

void madieva::show(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.circles.find(name);
  if (it == state.circles.end())
  {
    throw std::invalid_argument("the name of the circle does not exist");
  }
  std::cout << it->second << "\n";
}

void madieva::showSet(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.sets.find(name);
  if (it == state.sets.end())
  {
    throw std::invalid_argument("the name of the set does not exist");
  }
  for (std::size_t i = 0; i < it->second.size(); ++i)
  {
    std::cout << *(it->second[i]) << "\n";
  }
}

void madieva::frame(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.circles.find(name);
  if (it == state.circles.end())
  {
    throw std::invalid_argument("the name of the circle does not exist");
  }
  std::cout << madieva::computeFrame(it->second) << "\n";
}

void madieva::frameSet(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.sets.find(name);
  if (it == state.sets.end())
  {
    throw std::invalid_argument("the name of the set does not exist");
  }
  std::cout << madieva::computeFrame(it->second) << "\n";
}
