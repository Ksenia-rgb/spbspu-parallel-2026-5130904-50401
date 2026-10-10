#include <iostream>
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace kondrat
{
  long long parseArgument(const char * argument);
}

int main(int argc, char ** argv)
{
  if (argc != 3 && argc != 4)
  {
    std::cerr << "Usage: ./lab threads tries [seed]\n";
    return 1;
  }

  long long threads = 0;
  long long tries = 0;
  long long seed = 0;

  try
  {
    threads = kondrat::parseArgument(argv[1]);
    tries = kondrat::parseArgument(argv[2]);

    if (argc == 4)
    {
      seed = kondrat::parseArgument(argv[3]);
    }
  }
  catch (const std::invalid_argument & error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
  catch (const std::out_of_range & error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }

  if (threads < 0 || tries <= 0 || seed < 0)
  {
    std::cerr << "Threads and seed must be nonnegative, tries must be positive\n";
    return 1;
  }
}

long long kondrat::parseArgument(const char * argument)
{
  size_t position = 0;
  const long long value = std::stoll(argument, &position);

  if (argument[position] != '\0')
  {
    throw std::invalid_argument("Invalid argument");
  }

  return value;
}
