#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#include "area.hpp"
#include "input.hpp"

int main(int argc, char **argv)
{
  const int shortForm = 3;
  const int fullForm = 4;
  if ((argc != shortForm) && (argc != fullForm)) {
    std::cerr << "Usage: lab threads tries [seed]\n";
    return 1;
  }

  const int threadsPos = 1;
  const int triesPos = 2;
  const int seedPos = 3;
  std::size_t threads = 0;
  std::size_t tries = 0;
  std::size_t seed = 0;
  try {
    threads = petrov::parseCount(argv[threadsPos]);
    tries = petrov::parseCount(argv[triesPos]);
    if (argc == fullForm) {
      seed = petrov::parseCount(argv[seedPos]);
    }
  } catch (const std::exception &e) {
    std::cerr << "Bad argument: " << e.what() << '\n';
    return 1;
  }
  if (tries == 0) {
    std::cerr << "Number of tries must be positive\n";
    return 1;
  }
}
