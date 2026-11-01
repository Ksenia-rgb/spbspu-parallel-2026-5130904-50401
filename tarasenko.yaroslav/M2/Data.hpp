#ifndef TARASENKO_YAROSLAV_M2_DATA_HPP
#define TARASENKO_YAROSLAV_M2_DATA_HPP

#include <string>
#include <unordered_map>
#include "geometry.hpp"
#include "shared_memory.hpp"

namespace tarasenko {
  struct CircleSet
  {
    std::string memory_name;
    Circle* circles;
    size_t count;
  };

  struct Data
  {
    std::unordered_map< std::string, Circle > circles;
    std::unordered_map< std::string, CircleSet > sets;

    ~Data()
    {
      for (const auto& item: sets) {
        const CircleSet& set = item.second;
        freeSharedCircles(set.memory_name.c_str(), set.circles, set.count);
      }
    }
  };
}

#endif
