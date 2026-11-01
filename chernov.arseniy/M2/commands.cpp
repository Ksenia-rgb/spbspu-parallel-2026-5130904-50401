#include "commands.hpp"

#include <istream>
#include <limits>

std::istream & chernov::operator>>(std::istream & input, const CommandIO & cmd_io)
{
  std::istream::sentry sentry(input);
  if (!sentry) {
    return input;
  }

  std::string str = "";
  input >> str;

  if (input) {
    try {
      cmd_io.cmds.at(str)(input, cmd_io.output, cmd_io.ellipses);
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

void chernov::runCommands(std::istream & input, CommandIO & cmd_io)
{
  if (!(input >> cmd_io)) {
    return;
  }
  runCommands(input, cmd_io);
}

void chernov::cmdCircle(std::istream & input, std::ostream &, std::unordered_map< std::string, ellipse_t > & ellipses)
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

  ellipses.insert({name, ellipse_t{.center = point_t{.x = x, .y = y}, .horizontal_radius = r, .vertical_radius = r}});
}
