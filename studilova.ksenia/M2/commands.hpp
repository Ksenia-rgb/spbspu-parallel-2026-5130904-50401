#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iostream>
#include "storage.hpp"

namespace studilova
{
  void createCircle(std::istream& in, std::ostream& out, State& state);
  void showCircle(std::istream& in, std::ostream& out, State& state);
  void createSet(std::istream& in, std::ostream& out, State& state);
  void showSet(std::istream& in, std::ostream& out, State& state);
  void showFrame(std::istream& in, std::ostream& out, State& state);
  void showSetFrame(std::istream& in, std::ostream& out, State& state);
  void spawnProcess(std::istream& in, std::ostream& out, State& state);
  void areaOn(std::istream& in, std::ostream& out, State& state);
  void waitCalculation(std::istream& in, std::ostream& out, State& state);
  void showStatus(std::istream& in, std::ostream& out, State& state);
}

#endif
