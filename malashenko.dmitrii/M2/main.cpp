#include "application.hpp"
#include "commands.hpp"
#include <iostream>

int main()
{

  using namespace malashenko;
  using cmd_t = void (*)(std::istream&, std::ostream&, ApplicationState&);
  std::unordered_map<std::string, cmd_t> cmds;
  cmds["circle"] = doCircle;
  cmds["set"] = doSet;
  cmds["show"] = doShow;
  cmds["showset"] = doShowset;
  cmds["frame"] = doFrame;
  cmds["frameset"] = doFrameset;

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
    catch (...)
    {
      std::cout << "<INVALID COMMAND>\n";
      auto toignore = std::numeric_limits<std::streamsize>::max();
      std::cin.ignore(toignore, '\n');
    }
  }

  if (!std::cin.eof())
  {
    std::cerr << "Bad input\n";
    return 1;
  }
  return 0;
}