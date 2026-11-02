#include <iostream>
#include <limits>
#include <map>
#include <string>
#include "calc_manager.hpp"
#include "commands.hpp"

int main()
{
  namespace zub = zubarev;

  zub::setupSignals();

  using cmd_t = void (*)(std::istream&, std::ostream&, zub::Context&);
  std::map< std::string, cmd_t > cmds;

  cmds["circle"] = zub::cmd_circle;
  cmds["set"] = zub::cmd_set;
  cmds["show"] = zub::cmd_show;
  cmds["showset"] = zub::cmd_showset;
  cmds["frame"] = zub::cmd_frame;
  cmds["frameset"] = zub::cmd_frameset;
  cmds["spawn"] = zub::cmd_spawn;
  cmds["area-on"] = zub::cmd_area_on;
  cmds["status"] = zub::cmd_status;
  cmds["wait"] = zub::cmd_wait;

  zub::Context ctx;

  std::string cmd;
  while (std::cin >> cmd)
  {
    try
    {
      auto it = cmds.find(cmd);
      if (it == cmds.end())
      {
        throw std::out_of_range("unknown command");
      }
      it->second(std::cin, std::cout, ctx);
    }
    catch (const std::out_of_range&)
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      auto toignore = std::numeric_limits< std::streamsize >::max();
      std::cin.ignore(toignore, '\n');
    }
    catch (const std::logic_error&)
    {
      std::cout << "<INVALID COMMAND>\n";
    }
  }

  zub::cleanup(ctx.calcs);
  return 0;
}
