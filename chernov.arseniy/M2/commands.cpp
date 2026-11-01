#include "commands.hpp"

#include <istream>
#include <limits>

std::istream &chernov::operator>>(std::istream &input, const CommandIO &cmd_io)
{
  std::istream::sentry sentry(input);
  if (!sentry) {
    return input;
  }

  std::string str = "";
  input >> str;

  if (input) {
    try {
      cmd_io.cmds.at(str)(input, cmd_io.output, cmd_io.ellipses, cmd_io.sets);
    } catch (...) {
      input.setstate(std::ios::failbit);
    }
  }

  if (!input) {
    input.clear();
    input.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    if (cmd_io.cmds.find(str) != cmd_io.cmds.end()) {
      cmd_io.output << "<INVALID COMMAND>\n";
    } else {
      cmd_io.output << "<UNKNOWN COMMAND>\n";
    }
  }

  return input;
}

void chernov::runCommands(std::istream &input, CommandIO &cmd_io)
{
  if (!(input >> cmd_io)) {
    return;
  }
  runCommands(input, cmd_io);
}

void chernov::cmdCircle(std::istream &input, std::ostream &, ellipses_t &ellipses, sets_t &)
{
  std::string name;
  double r, x, y;
  if (!(input >> name >> r >> x >> y)) {
    throw std::runtime_error("invalid input");
  }

  if (ellipses.find(name) != ellipses.end()) {
    throw std::runtime_error("ellipse name already exists");
  }
  if (r <= 0) {
    throw std::runtime_error("ellipse radius must be greater than zero");
  }

  ellipses.insert(
      {name, ellipse_t{.center = point_t{.x = x, .y = y}, .horizontal_radius = r, .vertical_radius = r, .ksi = 0}});
}

void chernov::cmdSet(std::istream &input, std::ostream &, ellipses_t &ellipses, sets_t &sets)
{
  std::string name;
  long long size;
  if (!(input >> name >> size)) {
    throw std::runtime_error("invalid input");
  }

  if (sets.find(name) != sets.end()) {
    throw std::runtime_error("set name already exists");
  }
  if (size <= 0) {
    throw std::runtime_error("set size must be greater than zero");
  }

  set_t set;
  set.reserve(size);
  std::string shape;
  for (long long i = 0; i < size; ++i) {
    if (!(input >> shape)) {
      throw std::runtime_error("invalid input");
    }
    if (ellipses.find(shape) == ellipses.end()) {
      throw std::runtime_error("shape name not found");
    }

    set.push_back(shape);
  }

  sets.insert({name, std::move(set)});
}

void chernov::cmdShow(std::istream &input, std::ostream & output, ellipses_t &ellipses, sets_t &)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }

  if (ellipses.find(name) == ellipses.end()) {
    throw std::runtime_error("ellipse name not found");
  }

  output << ellipses.at(name) << "\n";
}

void chernov::cmdShowSet(std::istream &input, std::ostream & output, ellipses_t &ellipses, sets_t & sets)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }

  if (sets.find(name) == sets.end()) {
    throw std::runtime_error("set name not found");
  }

    const set_t & set = sets.at(name);
    output << ellipses.at(set[0]);
    for (size_t i = 1; i < set.size(); ++i) {
      output << "\n" << ellipses.at(set[i]);
    }
}
