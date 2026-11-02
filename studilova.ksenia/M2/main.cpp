#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include "commands.hpp"
#include "storage.hpp"

int main()
{
  studilova::circles_t circles;
  std::string command;

  while (std::cin >> command)
  {
    try
    {
      if (command == "circle")
      {
        studilova::createCircle(std::cin, circles);
      }
      else
      {
        std::cout << "<UNKNOWN COMMAND>\n";
      }
    }
    catch (const std::invalid_argument&)
    {
      std::cout << "<INVALID COMMAND>\n";
      std::cin.clear();
    }

    std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
  }
  return 0;
}
