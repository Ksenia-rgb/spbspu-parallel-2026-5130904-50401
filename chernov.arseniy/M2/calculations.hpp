#ifndef CALCULATION_HPP
#define CALCULATION_HPP

#include <ios>
#include <unordered_map>

namespace chernov {
  struct process_t {
    pid_t pid;
    size_t seed;
    int to_child[2];
    int to_parent[2];
  };

  using processes_t = std::unordered_map< std::string, process_t >;
}

#endif
