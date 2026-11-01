#include "commands.hpp"
#include "geometry.hpp"

#include <iostream>
#include <unordered_map>

int main()
{
  chernov::ellipses_t ellipses;
  chernov::sets_t sets;

  std::unordered_map< std::string, chernov::cmd_t > cmds;
  cmds["circle"] = chernov::cmdCircle;
  cmds["set"] = chernov::cmdSet;
  cmds["show"] = chernov::cmdShow;
  cmds["showset"] = chernov::cmdShowSet;
  cmds["frame"] = chernov::cmdFrame;

  auto cmd_io = chernov::CommandIO{std::cout, cmds, ellipses, sets};
  chernov::runCommands(std::cin, cmd_io);
}
