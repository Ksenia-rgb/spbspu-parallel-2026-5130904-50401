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
  void createSet(
    const std::vector< std::string > & words,
    const std::map< std::string, Shape > & shapes,
    std::map< std::string, std::vector< std::string > > & sets);
  void showSet(
    const std::vector< std::string > & words,
    const std::map< std::string, Shape > & shapes,
    const std::map< std::string, std::vector< std::string > > & sets,
    std::ostream & output);
  void showFrame(
    const std::vector< std::string > & words,
    const std::map< std::string, Shape > & shapes,
    std::ostream & output);
  void showSetFrame(
    const std::vector< std::string > & words,
    const std::map< std::string, Shape > & shapes,
    const std::map< std::string, std::vector< std::string > > & sets,
    std::ostream & output);
}

#endif
