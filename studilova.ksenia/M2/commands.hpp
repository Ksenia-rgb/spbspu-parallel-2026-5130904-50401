#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iostream>
#include "storage.hpp"

namespace studilova
{
  void createCircle(std::istream& in, circles_t& circles);
  void showCircle(std::istream& in, std::ostream& out, const circles_t& circles);
  void createSet(std::istream& in, const circles_t& circles, sets_t& sets);
}

#endif
