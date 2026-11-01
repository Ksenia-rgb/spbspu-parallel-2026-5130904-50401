#ifndef TARASENKO_YAROSLAV_M2_COMMANDS_HPP
#define TARASENKO_YAROSLAV_M2_COMMANDS_HPP

#include <iosfwd>

namespace tarasenko {
  void circle(std::istream& in, std::ostream& out);
  void set(std::istream& in, std::ostream& out);
  void show(std::istream& in, std::ostream& out);
  void showSet(std::istream& in, std::ostream& out);
  void frame(std::istream& in, std::ostream& out);
  void frameSet(std::istream& in, std::ostream& out);
  void spawn(std::istream& in, std::ostream& out);
  void areaOn(std::istream& in, std::ostream& out);
  void status(std::istream& in, std::ostream& out);
  void wait(std::istream& in, std::ostream& out);
}

#endif
