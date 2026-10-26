#include <string>
#include <cctype>
#include <vector>
#include <utility>
#include <unistd.h>
#include <system_error>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <poll.h>
#include "commands.hpp"
#include "ipc.hpp"

void vasyakin::cmd_circle(std::istream& in, std::ostream&, vasyakin::State& state)
{
  std::string circle_name;
  int r = 0, x = 0, y = 0;

  if (!(in >> circle_name >> r >> x >> y))
  {
    throw std::invalid_argument("invalid command");
  }

  if (circle_name.empty() || !std::isalpha(static_cast< unsigned char >(circle_name[0])))
  {
    throw std::invalid_argument("invalid command");
  }

  if (r <= 0)
  {
    throw std::invalid_argument("invalid command");
  }

  if (state.circles.count(circle_name))
  {
    throw std::invalid_argument("invalid command");
  }

  state.circles.insert({circle_name, circle_t{r, x, y}});
}

void vasyakin::cmd_set(std::istream& in, std::ostream&, vasyakin::State& state)
{
  std::string set_name;
  size_t set_size = 0;

  if (!(in >> set_name >> set_size))
  {
    throw std::invalid_argument("invalid command");
  }

  if (set_name.empty() || !std::isalpha(static_cast< unsigned char >(set_name[0])))
  {
    throw std::invalid_argument("invalid command");
  }

  if (set_size == 0)
  {
    throw std::invalid_argument("invalid command");
  }

  if (state.sets.count(set_name))
  {
    throw std::invalid_argument("invalid command");
  }

  vasyakin::ShapeSet ss;

  for (size_t i = 0; i < set_size; ++i)
  {
    std::string shape_name;
    if (!(in >> shape_name))
    {
      throw std::invalid_argument("invalid command");
    }

    if (!state.circles.count(shape_name))
    {
      throw std::invalid_argument("invalid command");
    }

    ss.shape_names.push_back(shape_name);
  }

  state.sets.insert({set_name, ss});
}
