#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "shapes.hpp"
#include <map>
#include <string>
#include <vector>

namespace zhuravleva {
  struct Storage {
    std::map< std::string, Shape > shapes;
  };
  bool addCircle(Storage& storage, const std::vector< std::string >& args);
  bool addEllipse(Storage& storage, const std::vector< std::string >& args);
  bool rotateShape(Storage& storage, const std::vector< std::string >& args);
}

#endif
