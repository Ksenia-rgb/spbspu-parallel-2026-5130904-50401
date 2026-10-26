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

void vasyakin::cmd_show(std::istream& in, std::ostream& out, vasyakin::State& state)
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

  const circle_t& circle = state.circles[shape_name];
  out << circle.r << " (" << circle.x << " " << circle.y << ")" << '\n';
}

void vasyakin::cmd_showset(std::istream& in, std::ostream& out, vasyakin::State& state)
{
  std::string set_name;

  if (!(in >> set_name))
  {
    throw std::invalid_argument("invalid command");
  }

  if (!state.sets.count(set_name))
  {
    throw std::invalid_argument("invalid command");
  }

  for (size_t i = 0; i < state.sets[set_name].shape_names.size(); ++i)
  {
    const circle_t& circle = state.circles.at(state.sets[set_name].shape_names[i]);
    out << circle.r << " (" << circle.x << " " << circle.y << ")" << '\n';
  }
}

void vasyakin::cmd_frame(std::istream& in, std::ostream& out, vasyakin::State& state)
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

  const circle_t& circle = state.circles[shape_name];

  std::pair< int, int > right = std::make_pair(circle.x + circle.r, circle.y + circle.r);
  std::pair< int, int > left = std::make_pair(circle.x - circle.r, circle.y - circle.r);

  out << "(" << left.first << " " << left.second << ")" << " (" << right.first << " " << right.second << ")" << '\n';
}

void vasyakin::cmd_frameset(std::istream& in, std::ostream& out, vasyakin::State& state)
{
  std::string set_name;

  if (!(in >> set_name))
  {
    throw std::invalid_argument("invalid command");
  }

  if (!state.sets.count(set_name))
  {
    throw std::invalid_argument("invalid command");
  }

  const ShapeSet& set = state.sets[set_name];
  if (set.shape_names.empty())
  {
    throw std::invalid_argument("invalid command");
  }

  const circle_t& circle_first = state.circles.at(set.shape_names[0]);

  std::pair< int, int > right = std::make_pair(circle_first.x + circle_first.r, circle_first.y + circle_first.r);
  std::pair< int, int > left = std::make_pair(circle_first.x - circle_first.r, circle_first.y - circle_first.r);

  for (size_t i = 1; i < set.shape_names.size(); ++i)
  {
    const circle_t& c = state.circles.at(set.shape_names[i]);

    right = {std::max(right.first, c.x + c.r), std::max(right.second, c.y + c.r)};
    left = {std::min(left.first, c.x - c.r), std::min(left.second, c.y - c.r)};
  }

  out << "(" << left.first << " " << left.second << ")" << " (" << right.first << " " << right.second << ")" << '\n';
}
