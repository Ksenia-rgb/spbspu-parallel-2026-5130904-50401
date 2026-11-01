#ifndef STATE_HPP
#define STATE_HPP

#include <map>
#include <iostream>
#include <vector>
#include "circle.hpp"

namespace madieva
{
  struct state_t
  {
    std::map< std::string, Circle > circles;
    std::map< std::string, std::vector< const Circle * > > sets;
  };
}

#endif
