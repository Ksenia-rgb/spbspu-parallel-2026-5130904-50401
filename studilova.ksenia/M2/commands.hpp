#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iostream>
#include "storage.hpp"
#include "process.hpp"

namespace studilova
{
  void createCircle(std::istream& in, circles_t& circles);
  void showCircle(std::istream& in, std::ostream& out, const circles_t& circles);
  void createSet(std::istream& in, const circles_t& circles, sets_t& sets);
  void showSet(std::istream& in, std::ostream& out, const circles_t& circles, const sets_t& sets);
  void showFrame(std::istream& in, std::ostream& out, const circles_t& circles);
  void showSetFrame(std::istream& in, std::ostream& out, const circles_t& circles, const sets_t& sets);
  void spawnProcess(std::istream& in, processes_t& processes);
  void areaOn(std::istream& in, const circles_t& circles, const sets_t& sets, processes_t& processes, calculations_t& calculations);
  void waitCalculation(std::istream& in, std::ostream& out, processes_t& processes, calculations_t& calculations);
  void showStatus(std::istream& in, std::ostream& out, processes_t& processes, calculations_t& calculations);
}

#endif
