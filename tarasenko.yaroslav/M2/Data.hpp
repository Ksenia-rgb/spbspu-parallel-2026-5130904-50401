#ifndef DATA_HPP
#define DATA_HPP

#include <string>
#include <unordered_map>
#include "../common/geometry.hpp"
#include "shared_memory.hpp"

namespace tarasenko {
  struct CircleSet
  {
    std::string memory_name;
    Ellipse* ellipses;
    size_t count;
  };

  struct Data
  {
    std::unordered_map< std::string, Ellipse > ellipses;
    std::unordered_map< std::string, CircleSet > sets;
    std::unordered_map< std::string, pid_t > processes;

    ~Data()
    {
      for (const auto& item: processes) {
        waitpid(item.second, nullptr, 0);
      }
      for (const auto& item: sets) {
        const CircleSet& set = item.second;
        freeSharedCircles(set.memory_name.c_str(), set.ellipses, set.count);
      }
    }
  };
}

#endif
