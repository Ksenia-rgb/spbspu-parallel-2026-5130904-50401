#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iostream>
#include <string>
#include <limits>

namespace levkin {

  inline void handleInvalidCommand(std::istream& is, std::ostream& os)
  {
    os << "<INVALID COMMAND>\n";
    is.clear();
    is.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
  }

  template< class DB >
  void cmd_circle(DB* db, std::istream& is, std::ostream& os)
  {
    std::string name;
    int r = 0, x = 0, y = 0;
    if (!(is >> name >> r >> x >> y) || r <= 0) {
      handleInvalidCommand(is, os);
      return;
    }
  }

}

#endif