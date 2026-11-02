#ifndef COMMANDS_HPP
#define COMMANDS_HPP
#include "calculations.hpp"
#include "geometry.hpp"

#include <functional>
#include <ios>
#include <iosfwd>
#include <unordered_map>

namespace chernov {
  struct App {
    ellipses_t &ellipses;
    sets_t &sets;
    processes_t &processes;
    calcs_t &calcs;
  };

  using cmd_t = std::function< void(std::istream &, std::ostream &, App &) >;
  struct CommandIO {
    std::ostream &output;
    const std::unordered_map< std::string, cmd_t > &cmds;
    App &app;
  };

  std::istream &operator>>(std::istream &input, const CommandIO &cmd_io);

  void runCommands(std::istream &input, CommandIO &cmd_io);

  void cmdCircle(std::istream &, std::ostream &, App &);
  void cmdSet(std::istream &, std::ostream &, App &);
  void cmdShow(std::istream &, std::ostream &, App &);
  void cmdShowSet(std::istream &, std::ostream &, App &);
  void cmdFrame(std::istream &, std::ostream &, App &);
  void cmdFrameSet(std::istream &, std::ostream &, App &);
  void cmdSpawn(std::istream &, std::ostream &, App &);
  void cmdAreaOn(std::istream &, std::ostream &, App &);
  void cmdStatus(std::istream &, std::ostream &, App &);
  void cmdWait(std::istream &, std::ostream &, App &);
}

#endif
