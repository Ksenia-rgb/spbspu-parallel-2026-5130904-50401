#ifndef COMMANDS_HPP
#define COMMANDS_HPP
#include "application.hpp"

namespace malashenko
{

  void doCircle(std::istream& in, std::ostream&, ApplicationState& state);
  void doSet(std::istream& in, std::ostream&, ApplicationState& state);
  void doShow(std::istream& in, std::ostream&, ApplicationState& state);
  void doShowset(std::istream& in, std::ostream&, ApplicationState& state);
  void doFrame(std::istream& in, std::ostream&, ApplicationState& state);
  void doFrameset(std::istream& in, std::ostream&, ApplicationState& state);

  void showCircle(const Circle& circle, std::ostream&);
  void showBox(const BoundingBox& box, std::ostream&);

}


#endif