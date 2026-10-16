#include <exception>
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
  size_t threads = 0;
  size_t tries = 0;
  size_t seed = 0;
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

  std::vector< petrov::circle_t > circles;
  try {
    circles = petrov::readCircles(std::cin);
  } catch (const std::invalid_argument &e) {
    std::cerr << "Bad input: " << e.what() << '\n';
    return 1;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 2;
  }

  try {
    const petrov::areas_t areas = petrov::estimateAreas(circles, threads, tries, seed);
    std::cout << std::setprecision(std::numeric_limits< double >::max_digits10);
    std::cout << areas.cover << ' ' << areas.comm << '\n';
  } catch (const std::exception &e) {
    std::cerr << "Calculation failed: " << e.what() << '\n';
    return 2;
  }
  return 0;
}
