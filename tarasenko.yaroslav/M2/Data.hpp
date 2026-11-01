#ifndef TARASENKO_YAROSLAV_M2_DATA_HPP
#define TARASENKO_YAROSLAV_M2_DATA_HPP

#include <string>
#include <unordered_map>
#include "geometry.hpp"

namespace tarasenko {
  struct Data
  {
    std::unordered_map< std::string, Circle > circles;
  };
}

#endif
