#include "application.hpp"
#include "commands.hpp"
#include <iostream>
#include <csignal>
#include <signal.h>

int main()
{

  struct sigaction action{};

  action.sa_handler = SIG_IGN;
  sigemptyset(&action.sa_mask);

  if (sigaction(SIGPIPE, &action, nullptr) == -1)
  {
    return 1;
  }

  using namespace malashenko;
  using cmd_t = void (*)(std::istream&, std::ostream&, ApplicationState&);
  std::unordered_map<std::string, cmd_t> cmds;
  cmds["circle"] = doCircle;
  cmds["set"] = doSet;
  cmds["show"] = doShow;
  cmds["showset"] = doShowset;
  cmds["frame"] = doFrame;
  cmds["frameset"] = doFrameset;
  cmds["spawn"] = doSpawn;
  cmds["area-on"] = doAreaOn;
  cmds["status"] = doStatus;
  cmds["wait"] = doWait;

  ApplicationState state;
  std::string cmd;
  while (std::cin >> cmd)
  {
    try
    {
      cmds.at(cmd)(std::cin, std::cout, state);
    }
    catch(const std::out_of_range&)
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      auto toignore = std::numeric_limits<std::streamsize>::max();
      std::cin.ignore(toignore, '\n');
    }
    catch (const std::exception& e)
    {
      std::cout << "<INVALID COMMAND>\n";
      std::cout << e.what() << '\n';
      auto toignore = std::numeric_limits<std::streamsize>::max();
      std::cin.ignore(toignore, '\n');
    }
  }
  for (auto it = state.processes.begin(); it != state.processes.end(); ++it)
  {
    closeProcess(it->second);
  }

  if (!std::cin.eof())
  {
    std::cerr << "Bad input\n";
    return 1;
  }
  return 0;
}