#include "commands.hpp"
#include <string>
#include <stdexcept>
#include"parser.hpp"

void madieva::addCircle(madieva::state_t & state, const madieva::Args & args) //logic_error
{
  if (args.size() != 5) {
    throw std::invalid_argument("incorrect number of arguments");
  }

  std::string name = args[1];
  long long r = parseInteger(args[2]);
  long long x = parseInteger(args[3]);
  long long y = parseInteger(args[4]);

  Circle circle(r, x, y);

  auto it = state.circles.find(name);
  if (it != state.circles.end())
  {
    throw std::invalid_argument("the name of the circle exists");
  }
  state.circles.insert({name, circle});
}
