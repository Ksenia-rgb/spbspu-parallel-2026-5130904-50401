#ifndef COMMANDS_HPP
#define COMMANDS_HPP
#include "geometry.hpp"

#include <functional>
#include <iosfwd>
#include <unordered_map>

namespace chernov {
  using cmd_t = std::function< void(std::istream &, std::ostream &, ellipses_t &, sets_t &) >;

  struct CommandIO {
    std::ostream &output;
    const std::unordered_map< std::string, cmd_t > &cmds;
    ellipses_t &ellipses;
    sets_t &sets;
  };

  std::istream &operator>>(std::istream &input, const CommandIO &cmd_io);

  void runCommands(std::istream & input, CommandIO & cmd_io);

  void cmdCircle(std::istream &, std::ostream &, ellipses_t &, sets_t &);
  void cmdSet(std::istream &, std::ostream &, ellipses_t &, sets_t &);
}

#endif
