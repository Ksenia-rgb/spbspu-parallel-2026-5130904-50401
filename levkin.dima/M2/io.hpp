#ifndef IO_HPP
#define IO_HPP

#include <iostream>
#include <array>
#include <algorithm>
#include <cassert>
#include <limits>
#include <cstring>

namespace levkin {

  template< class DB > struct IO {
    using fn_t = void (*)(DB*, std::istream& is, std::ostream& os);

    struct cmd_t {
      fn_t fn;
      const char* name;
    };

    template< std::size_t N >
    IO(std::istream& is, std::ostream& os, DB& db, const std::array< cmd_t, N >& cmds):
      _commands{},
      _count(N),
      _is(is),
      _os(os),
      _db(db)
    {
      static_assert(N <= 32, "Too many commands for internal buffer");
      std::copy(cmds.begin(), cmds.end(), _commands.begin());
    }

    void run(const char* name)
    {
      for (std::size_t i = 0; i < _count; ++i) {
        if (strcmp(name, _commands[i].name)) {
          _commands[i].fn(&_db, _is, _os);
          return;
        }
      }
      _os << "<UNKNOWN COMMAND>\n";

      _is.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }

  private:
    std::array< cmd_t, 32 > _commands;
    std::size_t _count = 0;
    std::istream& _is;
    std::ostream& _os;
    DB& _db;
  };

}

#endif
