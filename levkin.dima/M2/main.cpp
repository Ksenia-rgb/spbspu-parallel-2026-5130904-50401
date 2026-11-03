#include <iostream>
#include "io.hpp"
#include <vector>

struct DB {
  std::vector< int > processes;
} db;

void cmdCircle(DB*, std::istream& is, std::ostream& os)
{
  std::string dummy;
  is >> dummy;
  os << dummy << dummy << "\n";
}

void cmdRename(DB*, std::istream& is, std::ostream& os)
{
  std::string dummy;
  is >> dummy;
  os << dummy << dummy << "\n";
}
int main()
{
  using IO = levkin::IO< DB >;

  const std::array< IO::cmd_t, 2 > cmds = {{{&cmdCircle, "move"}, {&cmdRename, "rename"}}};

  IO io(std::cin, std::cout, db, cmds);
  const char* cmdd = "move" ;io.run(cmdd);

  std::cout << "levkin.dima\n" << sizeof(io);
}
