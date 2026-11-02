#include "commands.hpp"
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>

int main()
{
  std::unordered_map< std::string, hvostov::cmd_t > cmds;
  hvostov::info_t info;
  cmds["circle"] = hvostov::addCircle;
  cmds["ellipse"] = hvostov::addEllipse;
  cmds["rotate"] = hvostov::rotate;
  cmds["show"] = hvostov::show;
  cmds["set"] = hvostov::set;
  cmds["showset"] = hvostov::showSet;
  cmds["frame"] = hvostov::getFrame;
  cmds["frameset"] = hvostov::getFrameSet;
  cmds["spawn"] = hvostov::spawnProcess;
  cmds["area-on"] = hvostov::areaOn;
  cmds["status"] = hvostov::status;
  cmds["wait"] = hvostov::wait;
  cmds["timeout"] = hvostov::timeout;
  cmds["kill"] = hvostov::killProcess;

  std::string cmd;
  while (std::cin >> cmd) {
    auto it = cmds.find(cmd);
    if (it != cmds.end()) {
      try {
        it->second(std::cin, std::cout, info);
      } catch (...) {
        std::cout << "<INVALID COMMAND>\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
      }
    } else {
      std::cout << "<UNKNOWN COMMAND>\n";
      std::cin.clear();
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
  }
  return 0;
}
