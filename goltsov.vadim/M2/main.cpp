#include <csignal>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include "commands.hpp"
#include "shared_circles.hpp"

int main()
{
  std::signal(SIGPIPE, SIG_IGN);
  goltsov::SharedCircles shared_circles = goltsov::SharedCircles(goltsov::SHMN, 10);
  std::map< std::string, void (*)(goltsov::SharedCircles&) > commands;
  commands.insert({"circle", &goltsov::circle});
  commands.insert({"set", &goltsov::set});
  commands.insert({"show", &goltsov::show});
  commands.insert({"showset", &goltsov::showset});
  commands.insert({"frame", &goltsov::frame});
  commands.insert({"frameset", &goltsov::frameset});
  commands.insert({"spawn", &goltsov::spawn});
  commands.insert({"area-on", &goltsov::areas_on});
  commands.insert({"status", &goltsov::status});
  commands.insert({"wait", &goltsov::wait});

  std::string command;
  while (std::cin >> command)
  {
    if (!commands.count(command))
    {
      std::cout << "<UNKNOWN COMMAND>\n";
    }
    else
    {
      try
      {
        commands.at(command)(shared_circles);
      }
      catch (const std::exception&)
      {
        std::cout << "<INVALID COMMAND>\n";
      }
    }
    std::cin.clear();
    std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
  }
  goltsov::shutdown();
}
