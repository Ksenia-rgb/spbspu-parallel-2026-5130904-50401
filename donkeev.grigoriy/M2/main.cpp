#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "shape_manager.hpp"
#include "help_functions.hpp"

int main()
{
  donkeev::ShapeManager manager;
  std::string line;

  while (std::getline(std::cin, line))
  {
    if (line.empty())
    {
      continue;
    }

    auto tokens = donkeev::splitIntoTokens(line);
    if (donkeev::isUnknownCommand(tokens[0]))
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      continue;
    }
    if (!donkeev::isValidCommandTemplate(tokens))
    {
      std::cout << "<INVALID COMMAND>\n";
      continue;
    }
  }
}
