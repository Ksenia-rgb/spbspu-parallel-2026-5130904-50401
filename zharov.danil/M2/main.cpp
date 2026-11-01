#include <exception>
#include <iostream>
#include <signal.h>
#include <string>
#include <unordered_map>
#include "commands.hpp"

int main()
{
  struct sigaction ignore = {};
  ignore.sa_handler = SIG_IGN;
  if (sigaction(SIGPIPE, &ignore, nullptr) < 0)
  {
    std::cerr << "Can not ignore SIGPIPE\n";
    return 1;
  }

  using cmd = void (*)(std::ostream&, const zharov::Args&, zharov::Context&);
  using constCmd = void (*)(std::ostream&, const zharov::Args&, const zharov::Context&);
  std::unordered_map< std::string, cmd > cmds;
  std::unordered_map< std::string, constCmd > const_cmds;
  cmds["circle"] = zharov::createCircle;
  cmds["set"] = zharov::createSet;
  cmds["spawn"] = zharov::spawnProcess;
  cmds["area-on"] = zharov::startArea;
  cmds["status"] = zharov::showStatus;
  cmds["wait"] = zharov::waitCalc;
  const_cmds["show"] = zharov::showShape;
  const_cmds["showset"] = zharov::showSet;
  const_cmds["frame"] = zharov::showFrame;
  const_cmds["frameset"] = zharov::showFrameSet;

  zharov::Context ctx;
  std::string line;
  while (std::getline(std::cin, line))
  {
    const zharov::Args args = zharov::splitLine(line);
    if (args.empty())
    {
      continue;
    }
    const std::string& command = args[0];
    if (cmds.count(command) == 0 && const_cmds.count(command) == 0)
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      continue;
    }
    try
    {
      if (cmds.count(command) > 0)
      {
        cmds.at(command)(std::cout, args, ctx);
      }
      else
      {
        const_cmds.at(command)(std::cout, args, ctx);
      }
    }
    catch (const std::exception&)
    {
      std::cout << "<INVALID COMMAND>\n";
    }
  }
  ctx.workers.clear();
}
