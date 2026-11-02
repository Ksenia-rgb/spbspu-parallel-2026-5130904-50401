#include <iostream>
#include <string>
#include "calc_manager.hpp"
#include "commands.hpp"

int main()
{
  zubarev::setupSignals();
  zubarev::ShapesTable shapes;
  zubarev::CalcTable calcs;

  std::string line;
  while (std::getline(std::cin, line)) {
    zubarev::processCommand(line, shapes, calcs, std::cout);
  }

  zubarev::cleanup(calcs);
  return 0;
}
