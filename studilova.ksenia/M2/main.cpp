#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <signal.h>
#include <map>
#include "commands.hpp"
#include "storage.hpp"
#include "process.hpp"

int main()
{
  studilova::State state;
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
  {
    std::cerr << "Cannot ignore SIGPIPE\n";
    return 1;
  }

  using cmd_t = void (*)(std::istream&, std::ostream&, studilova::State&);
  std::map< std::string, cmd_t > commands;

  commands["circle"] = studilova::createCircle;
  commands["set"] = studilova::createSet;
  commands["show"] = studilova::showCircle;
  commands["showset"] = studilova::showSet;
  commands["frame"] = studilova::showFrame;
  commands["frameset"] = studilova::showSetFrame;
  commands["spawn"] = studilova::spawnProcess;
  commands["area-on"] = studilova::areaOn;
  commands["status"] = studilova::showStatus;
  commands["wait"] = studilova::waitCalculation;

  std::string command;

  while (std::cin >> command)
  {
    try
    {
      const auto found = commands.find(command);

      if (found == commands.end())
      {
        std::cout << "<UNKNOWN COMMAND>\n";
      }
      else
      {
        found->second(std::cin, std::cout, state);
      }
    }
    catch (const std::invalid_argument&)
    {
      std::cout << "<INVALID COMMAND>\n";
      std::cin.clear();
    }
    catch (const std::runtime_error& e)
    {
      std::cerr << e.what() << '\n';
      std::cin.clear();
    }

    std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
  }
  return studilova::finishProcesses(state.processes, state.calculations, std::cerr);
}
