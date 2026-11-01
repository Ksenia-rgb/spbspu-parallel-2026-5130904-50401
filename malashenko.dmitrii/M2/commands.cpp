#include "commands.hpp"
#include <iostream>


void malashenko::showCircle(const Circle& circle, std::ostream& out)
{
  out << circle.radius << " (" << circle.x << " " << circle.y << ")" << '\n';
}
void malashenko::showBox(const BoundingBox& box, std::ostream& out)
{
  out << "(" << box.min_x << " " << box.min_y << ")" << " (" << box.max_x << " " << box.max_y << ")" << '\n';
}

void malashenko::doCircle(std::istream& in, std::ostream&, ApplicationState& state)
{
  std::string name;
  int radius = 0, x = 0, y = 0;

  if (!(in >> name >> radius >> x >> y))
  {
    throw std::invalid_argument("Invalid arguments");
  }

  malashenko::createCircle(state, name, radius, x, y);
}

void malashenko::doSet(std::istream& in, std::ostream&, ApplicationState& state)
{
  std::string name;
  size_t size = 0;

  if (!(in >> name >> size))
  {
    throw std::invalid_argument("Invalid arguments");
  }

  if (size == 0)
  {
    throw std::invalid_argument("Invalid arguments");
  }


  std::vector< std::string > circle_names;
  circle_names.reserve(size);

  for (size_t i = 0; i < size; ++i)
  {
    std::string circle_name;
    if (!(in >> circle_name))
    {
      throw std::invalid_argument("Invalid circle name");
    }

    circle_names.push_back(circle_name);
  }

  createSet(state, name, circle_names);
}

void malashenko::doShow(std::istream& in, std::ostream& out, ApplicationState& state)
{
  std::string circle_name;

  if (!(in >> circle_name))
  {
    throw std::invalid_argument("Invalid circle name");
  }

  if (!state.circles.count(circle_name))
  {
    throw std::invalid_argument("Invalid circle name");
  }

  showCircle(getCircle(state, circle_name), out);
}

void malashenko::doShowset(std::istream& in, std::ostream& out, ApplicationState& state)
{
  std::string set_name;

  if (!(in >> set_name))
  {
    throw std::invalid_argument("Invalid set name");
  }

  if (!state.sets.count(set_name))
  {
    throw std::invalid_argument("Unknown set name");
  }

  const std::vector< std::string >& set = getSet(state, set_name);
  for (size_t i = 0; i < set.size(); ++i)
  {
    showCircle(getCircle(state, set[i]), out);
  }
}

void malashenko::doFrame(std::istream& in, std::ostream& out, ApplicationState& state)
{
  std::string circle_name;

  if (!(in >> circle_name))
  {
    throw std::invalid_argument("Invalid circle name");
  }

  if (!state.circles.count(circle_name))
  {
    throw std::invalid_argument("invalid command");
  }

  showBox(getCircleFrame(state, circle_name), out);
}

void malashenko::doFrameset(std::istream& in, std::ostream& out, ApplicationState& state)
{
  std::string set_name;

  if (!(in >> set_name))
  {
    throw std::invalid_argument("Invalid set name");
  }

  if (!state.sets.count(set_name))
  {
    throw std::invalid_argument("Unknown set name");
  }

  showBox(getSetFrame(state, set_name), out);
}


