#ifndef TARASENKO_YAROSLAV_M2_COMMANDS_HPP
#define TARASENKO_YAROSLAV_M2_COMMANDS_HPP

#include <iosfwd>
#include "Data.hpp"

namespace tarasenko {
  void circle(Data& context, std::istream& in, std::ostream& out);
  void set(Data& context, std::istream& in, std::ostream& out);
  void show(Data& context, std::istream& in, std::ostream& out);
  void showSet(Data& context, std::istream& in, std::ostream& out);
  void frame(Data& context, std::istream& in, std::ostream& out);
  void frameSet(Data& context, std::istream& in, std::ostream& out);
  void spawn(Data& context, std::istream& in, std::ostream& out);
  void areaOn(Data& context, std::istream& in, std::ostream& out);
  void status(Data& context, std::istream& in, std::ostream& out);
  void wait(Data& context, std::istream& in, std::ostream& out);
}

#endif
