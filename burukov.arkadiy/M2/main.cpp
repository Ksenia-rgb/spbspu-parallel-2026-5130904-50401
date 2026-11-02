#include <iostream>
#include <string>

#include <signal.h>

#include "commands.hpp"

int main(int argc, char ** argv)
{
  (void)argv;
  constexpr int BAD_EXIT = 1;
  constexpr int GOOD_EXIT = 0;

  if (argc != 1)
  {
    std::cerr << "No command line arguments allowed\n";
    return BAD_EXIT;
  }

  signal(SIGPIPE, SIG_IGN);

  burukov::CommandProcessor processor;
  int child_exit_flag = 0;

  for (std::string line; std::getline(std::cin, line);)
  {
    processor.processLine(line, child_exit_flag);
    if (child_exit_flag != 0)
    {
      return GOOD_EXIT;
    }
  }

  processor.shutdownAll();
  return GOOD_EXIT;
}
