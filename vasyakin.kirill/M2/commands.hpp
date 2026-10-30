#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iostream>
#include "structures.hpp"

namespace vasyakin
{
  void cmdCircle(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmdSet(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmdShow(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmdShowset(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmdFrame(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmdFrameset(std::istream& in, std::ostream& out, vasyakin::State& state);

  void cmdSpawn(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmdAreaon(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmdStatus(std::istream& in, std::ostream& out, vasyakin::State& state);
  void cmdWait(std::istream& in, std::ostream& out, vasyakin::State& state);
}

#endif
