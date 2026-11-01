#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iosfwd>
#include <map>
#include <string>
#include <vector>
#include "shape.hpp"

namespace kondrat
{
  void createShape(
    const std::vector< std::string > & words,
    std::map< std::string, Shape > & shapes);
  void showShape(
    const std::vector< std::string > & words,
    const std::map< std::string, Shape > & shapes,
    std::ostream & output);
  void rotateShape(
    const std::vector< std::string > & words,
    std::map< std::string, Shape > & shapes);
}

#endif
