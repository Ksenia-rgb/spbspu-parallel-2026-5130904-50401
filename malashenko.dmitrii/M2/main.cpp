#include <iostream>
#include <csignal>
#include <signal.h>
#include <unordered_map>
#include <limits>
#include "application.hpp"
#include "commands.hpp"
#include "string_buf.hpp"

int main()
{
  malashenko::ApplicationState state;

  using command_t = void (*)(std::istream&, std::ostream&, malashenko::ApplicationState&);

  std::unordered_map< std::string, command_t > commands;

  commands["circle"] = malashenko::doCircle;
  commands["set"] = malashenko::doSet;
  commands["show"] = malashenko::doShow;
  commands["showset"] = malashenko::doShowset;
  commands["frame"] = malashenko::doFrame;
  commands["frameset"] = malashenko::doFrameset;
  commands["spawn"] = malashenko::doSpawn;
  commands["area-on"] = malashenko::doAreaOn;
  commands["status"] = malashenko::doStatus;
  commands["wait"] = malashenko::doWait;

  std::string line;

  while (std::getline(std::cin, line))
  {
    if (!line.empty() && line.back() == '\r')
    {
      line.pop_back();
    }

    if (line.empty())
    {
      continue;
    }

    const size_t pos = line.find_first_of(" \t");

    const std::string command = (pos == std::string::npos) ? line : line.substr(0, pos);

    const std::string arguments = (pos == std::string::npos) ? "" : line.substr(pos);

    try
    {
      auto it = commands.find(command);

      if (it == commands.end())
      {
        std::cout << "<UNKNOWN COMMAND>\n";
        continue;
      }

      malashenko::StringBuf buffer(arguments);
      std::istream input(&buffer);

      it->second(input, std::cout, state);

      input >> std::ws;

      if (input.peek() != std::char_traits< char >::eof())
      {
        throw std::invalid_argument("Extra arguments");
      }
    }
    catch (const std::invalid_argument&)
    {
      std::cout << "<INVALID COMMAND>\n";
    }
    catch (const std::runtime_error& error)
    {
      std::cout << error.what() << '\n';
    }
  }

  return 0;
}

