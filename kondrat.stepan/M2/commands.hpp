#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <map>
#include <string>
#include <vector>
#include "shape.hpp"

namespace kondrat
{
  void createShape(
    const std::vector< std::string > & words,
    std::map< std::string, Shape > & shapes);
}

#endif
