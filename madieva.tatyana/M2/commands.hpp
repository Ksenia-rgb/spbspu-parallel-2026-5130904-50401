#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <functional>
#include <istream>
#include "state.hpp"
#include "parser.hpp"

namespace madieva
{
  using Command = std::function< void(madieva::state_t &, const madieva::Args &) >;

  void addCircle(madieva::state_t &, const madieva::Args &);
}

#endif
