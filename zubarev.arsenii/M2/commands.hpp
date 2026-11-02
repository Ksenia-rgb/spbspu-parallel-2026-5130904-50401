#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iosfwd>
#include <map>
#include <string>
#include <vector>
#include "shapes.hpp"

namespace zubarev {
  struct ShapesTable {
    std::map< std::string, Circle > circles;
    std::map< std::string, std::vector< std::string > > sets;
  };

  std::vector< std::string > splitLine(const std::string& line);
  bool isValidName(const std::string& name);
  bool isNameOccupied(const ShapesTable& table, const std::string& name);

  void cmd_circle(std::istream& in, std::ostream& out, ShapesTable& shapes);
  void cmd_set(std::istream& in, std::ostream& out, ShapesTable& shapes);
  void cmd_show(std::istream& in, std::ostream& out, ShapesTable& shapes);
  void cmd_showset(std::istream& in, std::ostream& out, ShapesTable& shapes);
  void cmd_frame(std::istream& in, std::ostream& out, ShapesTable& shapes);
  void cmd_frameset(std::istream& in, std::ostream& out, ShapesTable& shapes);
}

#endif
