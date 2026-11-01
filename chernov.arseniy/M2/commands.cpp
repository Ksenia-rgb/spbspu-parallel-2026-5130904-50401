#include "commands.hpp"

#include <iostream>
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
      cmd_io.cmds.at(str)(input, cmd_io.output, cmd_io.app);
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

void chernov::cmdCircle(std::istream &input, std::ostream &, App &app)
{
  std::string name;
  double r, x, y;
  if (!(input >> name >> r >> x >> y)) {
    throw std::runtime_error("invalid input");
  }

  if (app.ellipses.find(name) != app.ellipses.end()) {
    throw std::runtime_error("ellipse name already exists");
  }
  if (r <= 0) {
    throw std::runtime_error("ellipse radius must be greater than zero");
  }

  app.ellipses.insert(
      {name, ellipse_t{.center = point_t{.x = x, .y = y}, .horizontal_radius = r, .vertical_radius = r, .ksi = 0}});
}

void chernov::cmdSet(std::istream &input, std::ostream &, App &app)
{
  std::string name;
  long long size;
  if (!(input >> name >> size)) {
    throw std::runtime_error("invalid input");
  }

  if (app.sets.find(name) != app.sets.end()) {
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
    if (app.ellipses.find(shape) == app.ellipses.end()) {
      throw std::runtime_error("shape name not found");
    }

    set.push_back(shape);
  }

  app.sets.insert({name, std::move(set)});
}

void chernov::cmdShow(std::istream &input, std::ostream &output, App &app)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }

  if (app.ellipses.find(name) == app.ellipses.end()) {
    throw std::runtime_error("ellipse name not found");
  }

  output << app.ellipses.at(name) << "\n";
}

void chernov::cmdShowSet(std::istream &input, std::ostream &output, App &app)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }

  if (app.sets.find(name) == app.sets.end()) {
    throw std::runtime_error("set name not found");
  }

  const set_t &set = app.sets.at(name);
  for (std::string ellipse_name : set) {
    output << app.ellipses.at(ellipse_name) << "\n";
  }
}

void chernov::cmdFrame(std::istream &input, std::ostream &output, App &app)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }

  if (app.ellipses.find(name) == app.ellipses.end()) {
    throw std::runtime_error("ellipse name not found");
  }

  const frame_t frame = getFrame(app.ellipses.at(name));
  output << frame << "\n";
}

void chernov::cmdFrameSet(std::istream &input, std::ostream &output, App &app)
{
  std::string name;
  if (!(input >> name)) {
    throw std::runtime_error("invalid input");
  }

  if (app.sets.find(name) == app.sets.end()) {
    throw std::runtime_error("set name not found");
  }

  const set_t &set = app.sets.at(name);
  std::vector< ellipse_t > ellipses_vector;
  ellipses_vector.reserve(set.size());

  for (std::string ellipse_name : set) {
    ellipses_vector.push_back(app.ellipses.at(ellipse_name));
  }

  const frame_t frame = getFrame(ellipses_vector);
  output << frame << "\n";
}
