#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iostream>
#include "structures.hpp"

namespace vasyakin
{
  void cmd_circle(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmd_set(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmd_show(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmd_showset(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmd_frame(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmd_frameset(std::istream& in, std::ostream& out, vasyakin::State& state);

  void cmd_spawn(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmd_areaon(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmd_status(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmd_wait(std::istream& in, std::ostream& out, vasyakin::State& state);
}

#endif
