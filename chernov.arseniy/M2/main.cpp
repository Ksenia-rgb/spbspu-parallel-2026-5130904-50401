#include "commands.hpp"
#include "geometry.hpp"

#include <iostream>
#include <unordered_map>

int main()
{
  chernov::ellipses_t ellipses;
  chernov::sets_t sets;
  chernov::processes_t processes;

  chernov::App app{ellipses, sets, processes};

  std::unordered_map< std::string, chernov::cmd_t > cmds;
  cmds["circle"] = chernov::cmdCircle;
  cmds["set"] = chernov::cmdSet;
  cmds["show"] = chernov::cmdShow;
  cmds["showset"] = chernov::cmdShowSet;
  cmds["frame"] = chernov::cmdFrame;
  cmds["frameset"] = chernov::cmdFrameSet;
  cmds["spawn"] = chernov::cmdSpawn;

  auto cmd_io = chernov::CommandIO{std::cout, cmds, app};
  chernov::runCommands(std::cin, cmd_io);
}
