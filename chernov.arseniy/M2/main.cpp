#include "commands.hpp"
#include "geometry.hpp"

#include <iostream>
#include <unordered_map>

int main()
{
  std::unordered_map< std::string, chernov::ellipse_t > ellipses;

  std::unordered_map< std::string, chernov::cmd_t > cmds;
  cmds["circle"] = chernov::cmdCircle;

  auto cmd_io = chernov::CommandIO{std::cout, cmds, ellipses};
  chernov::runCommands(std::cin, cmd_io);
}
