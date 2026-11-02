#ifndef TARASENKO_YAROSLAV_M2_DATA_HPP
#define TARASENKO_YAROSLAV_M2_DATA_HPP

#include <string>
#include <unordered_map>
#include <sys/types.h>
#include <sys/wait.h>
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
    std::unordered_map< std::string, pid_t > processes;

    ~Data()
    {
      for (const auto& item: processes) {
        waitpid(item.second, nullptr, 0);
      }
      for (const auto& item: sets) {
        const CircleSet& set = item.second;
        freeSharedCircles(set.memory_name.c_str(), set.circles, set.count);
      }
    }
  };
}

#endif
